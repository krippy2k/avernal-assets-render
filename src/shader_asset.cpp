#include <avernal/assets_render/shader_asset.hpp>
#include <avernal/core/assert.hpp>
#include <fstream>
#include <sstream>

namespace avernal {

std::shared_ptr<Asset> ShaderAssetLoader::load(
    AssetId id,
    std::string_view path,
    const AssetMetadata& metadata
) {
    auto asset = std::make_shared<ShaderAsset>();
    
    // Check file extension to determine format
    std::string path_str{path};
    
    if (path_str.ends_with(".shader")) {
        // Custom shader format with multiple stages
        asset->source_ = parse_shader_file(path);
    } else {
        // Single-file shader - read as-is
        std::ifstream file{path_str};
        if (!file.is_open()) {
            return nullptr;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string code = buffer.str();
        
        // Determine shader stage by extension
        if (path_str.ends_with(".vert")) {
            asset->source_.vertex_code = std::move(code);
        } else if (path_str.ends_with(".frag")) {
            asset->source_.fragment_code = std::move(code);
        } else if (path_str.ends_with(".comp")) {
            asset->source_.compute_code = std::move(code);
        } else if (path_str.ends_with(".hlsl") || path_str.ends_with(".glsl")) {
            // Assume it's a combined shader file
            asset->source_.vertex_code = code;
            asset->source_.fragment_code = code;
        }
    }
    
    return asset;
}

void ShaderAssetLoader::unload(Asset* asset) {
    if (auto* shader_asset = dynamic_cast<ShaderAsset*>(asset)) {
        shader_asset->source_.vertex_code.clear();
        shader_asset->source_.fragment_code.clear();
        shader_asset->source_.compute_code.clear();
    }
}

bool ShaderAssetLoader::reload(Asset* asset) {
    if (auto* shader_asset = dynamic_cast<ShaderAsset*>(asset)) {
        const std::string path = shader_asset->path();
        
        ShaderSource new_source;
        if (path.ends_with(".shader")) {
            new_source = parse_shader_file(path);
        } else {
            std::ifstream file{path};
            if (!file.is_open()) {
                return false;
            }
            
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string code = buffer.str();
            
            if (path.ends_with(".vert")) {
                new_source.vertex_code = std::move(code);
            } else if (path.ends_with(".frag")) {
                new_source.fragment_code = std::move(code);
            } else if (path.ends_with(".comp")) {
                new_source.compute_code = std::move(code);
            } else {
                new_source.vertex_code = code;
                new_source.fragment_code = code;
            }
        }
        
        shader_asset->source_ = std::move(new_source);
        return true;
    }
    
    return false;
}

ShaderSource ShaderAssetLoader::parse_shader_file(std::string_view path) {
    ShaderSource source;
    
    std::ifstream file{std::string{path}};
    if (!file.is_open()) {
        return source;
    }
    
    // Custom shader file format:
    // #shader vertex
    // ... vertex shader code ...
    // #shader fragment
    // ... fragment shader code ...
    // #shader compute
    // ... compute shader code ...
    
    enum class ShaderType { None, Vertex, Fragment, Compute };
    ShaderType current_type = ShaderType::None;
    
    std::stringstream vertex_ss, fragment_ss, compute_ss;
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.starts_with("#shader")) {
            // Parse shader type
            if (line.find("vertex") != std::string::npos) {
                current_type = ShaderType::Vertex;
            } else if (line.find("fragment") != std::string::npos || 
                       line.find("pixel") != std::string::npos) {
                current_type = ShaderType::Fragment;
            } else if (line.find("compute") != std::string::npos) {
                current_type = ShaderType::Compute;
            } else {
                current_type = ShaderType::None;
            }
        } else {
            // Append to current shader
            switch (current_type) {
                case ShaderType::Vertex:
                    vertex_ss << line << '\n';
                    break;
                case ShaderType::Fragment:
                    fragment_ss << line << '\n';
                    break;
                case ShaderType::Compute:
                    compute_ss << line << '\n';
                    break;
                default:
                    break;
            }
        }
    }
    
    source.vertex_code = vertex_ss.str();
    source.fragment_code = fragment_ss.str();
    source.compute_code = compute_ss.str();
    
    return source;
}

} // namespace avernal
