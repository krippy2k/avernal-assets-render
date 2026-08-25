#include <avernal/assets_render/texture_asset.hpp>
#include <avernal/core/assert.hpp>
#include <avernal/rhi/rhi.hpp>

// Use stb_image from samples directory (should be centralized later)
#define STB_IMAGE_IMPLEMENTATION
#include "../../avernal-samples/samples/triangle-textured/stb_image.h"

#include <fstream>
#include <filesystem>

namespace avernal {

TextureAssetLoader::TextureAssetLoader(Device& device)
    : device_(&device) {
    AV_ASSERT(device_ != nullptr);
}

std::shared_ptr<Asset> TextureAssetLoader::load(
    AssetId id,
    std::string_view path,
    const AssetMetadata& metadata
) {
    namespace fs = std::filesystem;
    
    // Check if file exists
    if (!fs::exists(std::string{path})) {
        return nullptr;
    }
    
    // Load image data using stb_image
    int width, height, channels;
    stbi_uc* pixels = stbi_load(
        std::string{path}.c_str(),
        &width,
        &height,
        &channels,
        STBI_rgb_alpha  // Force 4 channels
    );
    
    if (!pixels) {
        return nullptr;
    }
    
    // Create texture asset
    auto asset = std::make_shared<TextureAsset>();
    asset->width_ = static_cast<std::uint32_t>(width);
    asset->height_ = static_cast<std::uint32_t>(height);
    asset->channels_ = 4;  // We forced RGBA
    
    // Create render texture
    asset->texture_ = render::Texture::create(
        *device_,
        asset->width_,
        asset->height_,
        Format::rgba8_unorm,
        pixels
    );
    
    // Free image data
    stbi_image_free(pixels);
    
    if (!asset->texture_) {
        return nullptr;
    }
    
    return asset;
}

void TextureAssetLoader::unload(Asset* asset) {
    if (auto* tex_asset = dynamic_cast<TextureAsset*>(asset)) {
        tex_asset->texture_.reset();
    }
}

bool TextureAssetLoader::reload(Asset* asset) {
    if (auto* tex_asset = dynamic_cast<TextureAsset*>(asset)) {
        const std::string path = tex_asset->path();
        
        // Load new image data
        int width, height, channels;
        stbi_uc* pixels = stbi_load(
            path.c_str(),
            &width,
            &height,
            &channels,
            STBI_rgb_alpha
        );
        
        if (!pixels) {
            return false;
        }
        
        // Create new texture
        auto new_texture = render::Texture::create(
            *device_,
            static_cast<std::uint32_t>(width),
            static_cast<std::uint32_t>(height),
            Format::rgba8_unorm,
            pixels
        );
        
        stbi_image_free(pixels);
        
        if (!new_texture) {
            return false;
        }
        
        // Replace old texture
        tex_asset->texture_ = std::move(new_texture);
        tex_asset->width_ = static_cast<std::uint32_t>(width);
        tex_asset->height_ = static_cast<std::uint32_t>(height);
        
        return true;
    }
    
    return false;
}

} // namespace avernal
