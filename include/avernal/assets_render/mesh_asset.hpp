#pragma once

#include <avernal/assets/asset_handle.hpp>
#include <avernal/assets/asset_loader.hpp>
#include <avernal/render/mesh.hpp>
#include <memory>
#include <string>
#include <vector>

namespace avernal {

// Forward declarations
class Device;

/// Vertex data for mesh loading
struct MeshVertexData {
    std::vector<render::Vertex> vertices;
    std::vector<std::uint32_t> indices;
};

/// Asset wrapper for render meshes
class MeshAsset : public Asset {
public:
    MeshAsset() = default;
    ~MeshAsset() override = default;
    
    /// Get the underlying render mesh
    [[nodiscard]] render::Mesh* mesh() noexcept { return mesh_.get(); }
    [[nodiscard]] const render::Mesh* mesh() const noexcept { return mesh_.get(); }
    
    /// Get mesh statistics
    [[nodiscard]] std::uint32_t vertex_count() const noexcept { return vertex_count_; }
    [[nodiscard]] std::uint32_t index_count() const noexcept { return index_count_; }

private:
    friend class MeshAssetLoader;
    std::unique_ptr<render::Mesh> mesh_;
    std::uint32_t vertex_count_{0};
    std::uint32_t index_count_{0};
};

/// Loader for mesh assets
class MeshAssetLoader : public IAssetLoader {
public:
    explicit MeshAssetLoader(Device& device);
    ~MeshAssetLoader() override = default;
    
    [[nodiscard]] AssetType type() const noexcept override {
        return AssetType::mesh;
    }
    
    [[nodiscard]] std::vector<std::string> extensions() const override {
        return {".obj", ".mesh"};
    }
    
    [[nodiscard]] std::shared_ptr<Asset> load(
        AssetId id,
        std::string_view path,
        const AssetMetadata& metadata
    ) override;
    
    void unload(Asset* asset) override;
    bool reload(Asset* asset) override;

private:
    /// Load OBJ file format
    [[nodiscard]] MeshVertexData load_obj(std::string_view path);
    
    Device* device_;
};

} // namespace avernal
