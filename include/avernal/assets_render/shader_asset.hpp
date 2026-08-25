#pragma once

#include <avernal/assets/asset_handle.hpp>
#include <avernal/assets/asset_loader.hpp>
#include <memory>
#include <string>
#include <vector>

namespace avernal {

// Forward declarations
namespace rhi { class Device; }

/// Shader source code
struct ShaderSource {
    std::string vertex_code;
    std::string fragment_code;
    std::string compute_code;
};

/// Asset wrapper for shaders
class ShaderAsset : public Asset {
public:
    ShaderAsset() = default;
    ~ShaderAsset() override = default;
    
    /// Get shader source
    [[nodiscard]] const ShaderSource& source() const noexcept { return source_; }
    
    /// Check shader type
    [[nodiscard]] bool has_vertex() const noexcept { return !source_.vertex_code.empty(); }
    [[nodiscard]] bool has_fragment() const noexcept { return !source_.fragment_code.empty(); }
    [[nodiscard]] bool has_compute() const noexcept { return !source_.compute_code.empty(); }

private:
    friend class ShaderAssetLoader;
    ShaderSource source_;
};

/// Loader for shader assets
class ShaderAssetLoader : public IAssetLoader {
public:
    ShaderAssetLoader() = default;
    ~ShaderAssetLoader() override = default;
    
    [[nodiscard]] AssetType type() const noexcept override {
        return AssetType::shader;
    }
    
    [[nodiscard]] std::vector<std::string> extensions() const override {
        return {".shader", ".hlsl", ".glsl", ".vert", ".frag", ".comp"};
    }
    
    [[nodiscard]] std::shared_ptr<Asset> load(
        AssetId id,
        std::string_view path,
        const AssetMetadata& metadata
    ) override;
    
    void unload(Asset* asset) override;
    bool reload(Asset* asset) override;

private:
    /// Parse shader file (custom .shader format)
    [[nodiscard]] ShaderSource parse_shader_file(std::string_view path);
};

} // namespace avernal
