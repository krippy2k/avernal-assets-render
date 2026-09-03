#pragma once

#include <avernal/assets/asset_handle.hpp>
#include <avernal/assets/asset_loader.hpp>
#include <avernal/render/texture.hpp>
#include <memory>
#include <string>

namespace avernal {

// Forward declarations
class Device;
class Renderer;

/// Asset wrapper for render textures
class TextureAsset : public Asset {
public:
    TextureAsset() = default;
    ~TextureAsset() override = default;
    
    /// Get the underlying render texture
    [[nodiscard]] render::Texture* texture() noexcept { return texture_.get(); }
    [[nodiscard]] const render::Texture* texture() const noexcept { return texture_.get(); }
    
    /// Get texture dimensions
    [[nodiscard]] std::uint32_t width() const noexcept { return width_; }
    [[nodiscard]] std::uint32_t height() const noexcept { return height_; }
    [[nodiscard]] std::uint32_t channels() const noexcept { return channels_; }

private:
    friend class TextureAssetLoader;
    std::unique_ptr<render::Texture> texture_;
    std::uint32_t width_{0};
    std::uint32_t height_{0};
    std::uint32_t channels_{0};
};

/// Loader for texture assets
class TextureAssetLoader : public IAssetLoader {
public:
    explicit TextureAssetLoader(Device& device);
    ~TextureAssetLoader() override = default;
    
    [[nodiscard]] AssetType type() const noexcept override {
        return AssetType::texture;
    }
    
    [[nodiscard]] std::vector<std::string> extensions() const override {
        return {".png", ".jpg", ".jpeg", ".bmp", ".tga", ".avtex"};
    }
    
    [[nodiscard]] std::shared_ptr<Asset> load(
        AssetId id,
        std::string_view path,
        const AssetMetadata& metadata
    ) override;
    
    void unload(Asset* asset) override;
    bool reload(Asset* asset) override;

private:
    Device* device_;
};

} // namespace avernal
