#include <avernal/assets_render/material_asset.hpp>
#include <avernal/assets_render/avmat.hpp>
#include <avernal/assets/asset_manager.hpp>
#include <avernal/core/assert.hpp>
#include <avernal/rhi/rhi.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace avernal {
namespace {

[[nodiscard]] std::filesystem::path resolve_asset_path(
    std::string_view owner_path, std::string_view referenced) {
    const std::filesystem::path ref{std::string{referenced}};
    if (ref.is_absolute()) {
        return ref;
    }
    const auto parent = std::filesystem::path{std::string{owner_path}}.parent_path();
    if (!parent.empty()) {
        const auto relative = parent / ref;
        if (std::filesystem::exists(relative)) {
            return relative;
        }
    }
    return ref;
}

}  // namespace

MaterialAssetLoader::MaterialAssetLoader(Device& device, AssetManager& asset_manager)
    : device_(&device), asset_manager_(&asset_manager) {
    AV_ASSERT(device_ != nullptr);
    AV_ASSERT(asset_manager_ != nullptr);
}

std::shared_ptr<Asset> MaterialAssetLoader::create_material(std::string_view owner_path,
    const Color& color, bool use_texture, bool use_3d, bool use_depth, std::string_view texture_path,
    bool two_sided) {
    auto asset = std::make_shared<MaterialAsset>();
    if (use_texture && !texture_path.empty()) {
        const auto resolved = resolve_asset_path(owner_path, texture_path);
        asset->texture_ = asset_manager_->load<TextureAsset>(resolved.generic_string());
        if (!asset->texture_) {
            asset->texture_ = asset_manager_->load<TextureAsset>(texture_path);
        }
    }

    GraphicsPipelineDesc pipeline_desc{};
    pipeline_desc.color = color;
    pipeline_desc.use_texture = use_texture && asset->texture_ && asset->texture_->texture();
    pipeline_desc.use_3d = use_3d;
    pipeline_desc.use_depth = use_depth;
    pipeline_desc.two_sided = two_sided;
    asset->material_ = render::Material::create(*device_, pipeline_desc);
    if (!asset->material_) {
        return nullptr;
    }
    if (pipeline_desc.use_texture) {
        asset->material_->set_texture(asset->texture_->texture());
    }
    return asset;
}

std::shared_ptr<Asset> MaterialAssetLoader::load(
    [[maybe_unused]] AssetId id, std::string_view path, [[maybe_unused]] const AssetMetadata& metadata) {
    const std::filesystem::path file_path{std::string{path}};
    if (file_path.extension() == ".avmat") {
        const auto document = load_avmat(file_path);
        if (!document) {
            return nullptr;
        }
        const Color color{document->color[0], document->color[1], document->color[2], document->color[3]};
        const bool use_texture = (document->flags & avmat_flag_use_texture) != 0;
        const bool use_3d = (document->flags & avmat_flag_use_3d) != 0;
        const bool use_depth = (document->flags & avmat_flag_use_depth) != 0;
        const bool two_sided = (document->flags & avmat_flag_two_sided) != 0;
        return create_material(
            path, color, use_texture, use_3d, use_depth, document->texture_path, two_sided);
    }

    std::ifstream file{std::string{path}};
    if (!file.is_open()) {
        return nullptr;
    }

    std::string texture_path;
    Color color = Color::white();
    bool use_3d = false;
    bool use_texture = false;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        if (iss >> key) {
            if (key == "texture:") {
                iss >> texture_path;
                use_texture = true;
            } else if (key == "color:") {
                iss >> color.r >> color.g >> color.b >> color.a;
            } else if (key == "use_3d:") {
                std::string value;
                iss >> value;
                use_3d = (value == "true" || value == "1");
            }
        }
    }

    return create_material(path, color, use_texture, use_3d, use_3d, texture_path);
}

void MaterialAssetLoader::unload(Asset* asset) {
    if (auto* mat_asset = dynamic_cast<MaterialAsset*>(asset)) {
        mat_asset->material_.reset();
        mat_asset->texture_.reset();
    }
}

bool MaterialAssetLoader::reload([[maybe_unused]] Asset* asset) {
    return false;
}

}  // namespace avernal
