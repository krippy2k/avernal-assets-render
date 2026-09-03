#include <avernal/assets_render/texture_asset.hpp>
#include <avernal/assets_render/avtex.hpp>
#include <avernal/assets_render/image.hpp>
#include <avernal/core/assert.hpp>
#include <avernal/rhi/rhi.hpp>

#include <filesystem>

namespace avernal {

TextureAssetLoader::TextureAssetLoader(Device& device)
    : device_(&device) {
    AV_ASSERT(device_ != nullptr);
}

std::shared_ptr<Asset> TextureAssetLoader::load(
    [[maybe_unused]] AssetId id, std::string_view path, [[maybe_unused]] const AssetMetadata& metadata) {
    namespace fs = std::filesystem;
    const fs::path file_path{std::string{path}};
    if (!fs::exists(file_path)) {
        return nullptr;
    }

    std::uint32_t width = 0;
    std::uint32_t height = 0;
    const std::uint8_t* pixels = nullptr;
    std::vector<std::uint8_t> avtex_pixels;

    if (file_path.extension() == ".avtex") {
        const auto image = load_avtex(file_path);
        if (!image) {
            return nullptr;
        }
        width = image->width;
        height = image->height;
        avtex_pixels = image->pixels;
        pixels = avtex_pixels.data();
    } else {
        const auto image = load_image_rgba8(file_path);
        if (!image) {
            return nullptr;
        }
        width = image->width;
        height = image->height;
        avtex_pixels = image->pixels;
        pixels = avtex_pixels.data();
    }

    auto asset = std::make_shared<TextureAsset>();
    asset->width_ = width;
    asset->height_ = height;
    asset->channels_ = 4;
    asset->texture_ = render::Texture::create(*device_, width, height, Format::rgba8_unorm, pixels);
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
        auto reloaded = load(tex_asset->id(), tex_asset->path(), {});
        if (!reloaded) {
            return false;
        }
        auto* next = static_cast<TextureAsset*>(reloaded.get());
        tex_asset->texture_ = std::move(next->texture_);
        tex_asset->width_ = next->width_;
        tex_asset->height_ = next->height_;
        tex_asset->channels_ = next->channels_;
        return true;
    }
    return false;
}

}  // namespace avernal
