#include <avernal/assets_render/model_asset.hpp>
#include <avernal/assets/asset_manager.hpp>
#include <avernal/core/assert.hpp>

#include <cstring>
#include <filesystem>

namespace avernal {
namespace {

[[nodiscard]] std::string resolve_asset_path(std::string_view owner_path, std::string_view referenced) {
    const std::filesystem::path ref{std::string{referenced}};
    if (ref.is_absolute()) {
        return ref.generic_string();
    }
    const auto parent = std::filesystem::path{std::string{owner_path}}.parent_path();
    if (!parent.empty()) {
        const auto relative = (parent / ref).lexically_normal();
        if (std::filesystem::exists(relative)) {
            return relative.generic_string();
        }
    }
    return ref.generic_string();
}

}  // namespace

ModelAssetLoader::ModelAssetLoader([[maybe_unused]] Device& device, AssetManager& asset_manager)
    : asset_manager_(&asset_manager) {
    AV_ASSERT(asset_manager_ != nullptr);
}

std::shared_ptr<Asset> ModelAssetLoader::load(
    [[maybe_unused]] AssetId id, std::string_view path, [[maybe_unused]] const AssetMetadata& metadata) {
    const auto document = load_avmodel(std::filesystem::path{std::string{path}});
    if (!document) {
        return nullptr;
    }

    auto asset = std::make_shared<ModelAsset>();
    asset->nodes_.resize(document->nodes.size());
    for (std::size_t i = 0; i < document->nodes.size(); ++i) {
        const auto& src = document->nodes[i];
        auto& dst = asset->nodes_[i];
        dst.parent = src.parent;
        dst.name = src.name;
        std::memcpy(dst.position, src.position, sizeof(dst.position));
        std::memcpy(dst.rotation, src.rotation, sizeof(dst.rotation));
        std::memcpy(dst.scale, src.scale, sizeof(dst.scale));
    }

    std::vector<AssetHandle<MeshAsset>> meshes;
    meshes.reserve(document->meshes.size());
    for (const auto& mesh : document->meshes) {
        meshes.push_back(asset_manager_->load<MeshAsset>(resolve_asset_path(path, mesh.path)));
    }

    std::vector<AssetHandle<MaterialAsset>> materials;
    materials.reserve(document->materials.size());
    for (const auto& material : document->materials) {
        materials.push_back(asset_manager_->load<MaterialAsset>(resolve_asset_path(path, material.path)));
    }

    asset->parts_.resize(document->parts.size());
    for (std::size_t i = 0; i < document->parts.size(); ++i) {
        const auto& src = document->parts[i];
        auto& dst = asset->parts_[i];
        dst.node = src.node;
        if (src.mesh != avmodel_none && src.mesh < meshes.size()) {
            dst.mesh = meshes[src.mesh];
        }
        if (src.material != avmodel_none && src.material < materials.size()) {
            dst.material = materials[src.material];
        }
    }
    return asset;
}

void ModelAssetLoader::unload(Asset* asset) {
    if (auto* model = dynamic_cast<ModelAsset*>(asset)) {
        model->parts_.clear();
        model->nodes_.clear();
    }
}

bool ModelAssetLoader::reload([[maybe_unused]] Asset* asset) {
    return false;
}

}  // namespace avernal
