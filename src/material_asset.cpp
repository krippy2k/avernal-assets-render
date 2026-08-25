#include <avernal/assets_render/material_asset.hpp>
#include <avernal/assets/asset_manager.hpp>
#include <avernal/core/assert.hpp>
#include <avernal/rhi/rhi.hpp>
#include <fstream>
#include <sstream>

namespace avernal {

MaterialAssetLoader::MaterialAssetLoader(Device& device, AssetManager& asset_manager)
    : device_(&device), asset_manager_(&asset_manager) {
    AV_ASSERT(device_ != nullptr);
    AV_ASSERT(asset_manager_ != nullptr);
}

std::shared_ptr<Asset> MaterialAssetLoader::load(
    AssetId id,
    std::string_view path,
    const AssetMetadata& metadata
) {
    // Load material definition file
    std::ifstream file{std::string{path}};
    if (!file.is_open()) {
        return nullptr;
    }
    
    auto asset = std::make_shared<MaterialAsset>();
    
    // Simple material file format:
    // texture: path/to/texture.png
    // color: r g b a
    // use_3d: true/false
    
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
    
    // Load texture if specified
    if (use_texture && !texture_path.empty()) {
        asset->texture_ = asset_manager_->load<TextureAsset>(texture_path);
    }
    
    // Create pipeline description
    GraphicsPipelineDesc pipeline_desc{};
    pipeline_desc.color = color;
    pipeline_desc.use_texture = use_texture;
    pipeline_desc.use_3d = use_3d;
    
    // Create material
    asset->material_ = render::Material::create(*device_, pipeline_desc);
    
    if (!asset->material_) {
        return nullptr;
    }
    
    // Set texture if available
    if (asset->texture_ && asset->texture_->texture()) {
        asset->material_->set_texture(asset->texture_->texture());
    }
    
    return asset;
}

void MaterialAssetLoader::unload(Asset* asset) {
    if (auto* mat_asset = dynamic_cast<MaterialAsset*>(asset)) {
        mat_asset->material_.reset();
        mat_asset->texture_.reset();
    }
}

bool MaterialAssetLoader::reload(Asset* asset) {
    // TODO: Implement material reloading
    return false;
}

} // namespace avernal
