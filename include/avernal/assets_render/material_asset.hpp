#pragma once

#include <avernal/assets/asset_handle.hpp>
#include <avernal/assets/asset_loader.hpp>
#include <avernal/render/material.hpp>
#include <avernal/assets_render/texture_asset.hpp>
#include <memory>
#include <string>

namespace avernal {

// Forward declarations
class Device;
class AssetManager;

/// Asset wrapper for render materials
class MaterialAsset : public Asset {
public:
    MaterialAsset() = default;
    ~MaterialAsset() override = default;
    
    /// Get the underlying render material
    [[nodiscard]] render::Material* material() noexcept { return material_.get(); }
    [[nodiscard]] const render::Material* material() const noexcept { return material_.get(); }
    
    /// Get the texture used by this material (if any)
    [[nodiscard]] AssetHandle<TextureAsset> texture() const noexcept { return texture_; }

private:
    friend class MaterialAssetLoader;
    std::unique_ptr<render::Material> material_;
    AssetHandle<TextureAsset> texture_;
};

/// Loader for material assets
class MaterialAssetLoader : public IAssetLoader {
public:
    MaterialAssetLoader(Device& device, AssetManager& asset_manager);
    ~MaterialAssetLoader() override = default;
    
    [[nodiscard]] AssetType type() const noexcept override {
        return AssetType::material;
    }
    
    [[nodiscard]] std::vector<std::string> extensions() const override {
        return {".mat", ".material", ".avmat"};
    }
    
    [[nodiscard]] std::shared_ptr<Asset> load(
        AssetId id,
        std::string_view path,
        const AssetMetadata& metadata
    ) override;
    
    void unload(Asset* asset) override;
    bool reload(Asset* asset) override;

private:
    [[nodiscard]] std::shared_ptr<Asset> create_material(std::string_view owner_path, const Color& color,
        bool use_texture, bool use_3d, bool use_depth, std::string_view texture_path,
        bool two_sided = false);

    Device* device_;
    AssetManager* asset_manager_;
};

} // namespace avernal
