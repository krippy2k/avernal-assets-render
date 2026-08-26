#pragma once

#include <avernal/assets/asset_handle.hpp>
#include <avernal/assets/asset_loader.hpp>
#include <avernal/render/avmesh.hpp>
#include <avernal/render/mesh.hpp>

#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace avernal {

class Device;

[[nodiscard]] std::optional<render::MeshGeometry> load_avmesh(const std::filesystem::path& path);
[[nodiscard]] bool save_avmesh(
    const std::filesystem::path& path, const render::MeshGeometry& geometry);

struct MeshVertexData {
    std::vector<render::Vertex> vertices;
    std::vector<std::uint32_t> indices;
};

class MeshAsset : public Asset {
public:
    MeshAsset() = default;
    ~MeshAsset() override = default;

    [[nodiscard]] render::Mesh* mesh() noexcept { return mesh_.get(); }
    [[nodiscard]] const render::Mesh* mesh() const noexcept { return mesh_.get(); }

    [[nodiscard]] std::uint32_t vertex_count() const noexcept { return vertex_count_; }
    [[nodiscard]] std::uint32_t index_count() const noexcept { return index_count_; }
    [[nodiscard]] render::IndexFormat index_format() const noexcept;
    [[nodiscard]] std::span<const render::Submesh> submeshes() const noexcept;
    [[nodiscard]] const render::Bounds* bounds() const noexcept;

private:
    friend class MeshAssetLoader;
    std::unique_ptr<render::Mesh> mesh_{};
    std::uint32_t vertex_count_{0};
    std::uint32_t index_count_{0};
};

class MeshAssetLoader : public IAssetLoader {
public:
    explicit MeshAssetLoader(Device& device);
    ~MeshAssetLoader() override = default;

    [[nodiscard]] AssetType type() const noexcept override { return AssetType::mesh; }

    [[nodiscard]] std::vector<std::string> extensions() const override {
        return {".avmesh", ".obj", ".mesh"};
    }

    [[nodiscard]] std::shared_ptr<Asset> load(
        AssetId id, std::string_view path, const AssetMetadata& metadata) override;

    void unload(Asset* asset) override;
    bool reload(Asset* asset) override;

private:
    [[nodiscard]] MeshVertexData load_obj(std::string_view path);
    [[nodiscard]] std::shared_ptr<MeshAsset> make_asset(const render::MeshGeometry& geometry);

    Device* device_{};
};

}  // namespace avernal
