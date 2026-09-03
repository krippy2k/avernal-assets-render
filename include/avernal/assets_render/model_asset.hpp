#pragma once

#include <avernal/assets/asset_handle.hpp>
#include <avernal/assets/asset_loader.hpp>
#include <avernal/assets_render/avmodel.hpp>
#include <avernal/assets_render/material_asset.hpp>
#include <avernal/assets_render/mesh_asset.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace avernal {

class Device;
class AssetManager;

struct ModelNodeInstance {
    std::uint32_t parent{avmodel_none};
    std::string name{};
    float position[3]{};
    float rotation[4]{0.0f, 0.0f, 0.0f, 1.0f};
    float scale[3]{1.0f, 1.0f, 1.0f};
};

struct ModelPartInstance {
    std::uint32_t node{avmodel_none};
    AssetHandle<MeshAsset> mesh{};
    AssetHandle<MaterialAsset> material{};
};

class ModelAsset : public Asset {
public:
    ModelAsset() = default;
    ~ModelAsset() override = default;

    [[nodiscard]] const std::vector<ModelNodeInstance>& nodes() const noexcept { return nodes_; }
    [[nodiscard]] const std::vector<ModelPartInstance>& parts() const noexcept { return parts_; }

private:
    friend class ModelAssetLoader;
    std::vector<ModelNodeInstance> nodes_{};
    std::vector<ModelPartInstance> parts_{};
};

class ModelAssetLoader : public IAssetLoader {
public:
    ModelAssetLoader(Device& device, AssetManager& asset_manager);
    ~ModelAssetLoader() override = default;

    [[nodiscard]] AssetType type() const noexcept override { return AssetType::model; }

    [[nodiscard]] std::vector<std::string> extensions() const override { return {".avmodel"}; }

    [[nodiscard]] std::shared_ptr<Asset> load(
        AssetId id, std::string_view path, const AssetMetadata& metadata) override;

    void unload(Asset* asset) override;
    bool reload(Asset* asset) override;

private:
    AssetManager* asset_manager_{};
};

}  // namespace avernal
