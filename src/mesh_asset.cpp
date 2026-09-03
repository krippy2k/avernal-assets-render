#include <avernal/assets_render/mesh_asset.hpp>
#include <avernal/core/assert.hpp>
#include <avernal/rhi/rhi.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace avernal {

std::optional<render::MeshGeometry> load_avmesh(const std::filesystem::path& path) {
    std::ifstream file{path, std::ios::binary | std::ios::ate};
    if (!file) {
        return std::nullopt;
    }

    const auto size = static_cast<std::size_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    std::vector<std::byte> bytes(size);
    if (size > 0 &&
        !file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size))) {
        return std::nullopt;
    }
    return render::read_avmesh(bytes);
}

bool save_avmesh(const std::filesystem::path& path, const render::MeshGeometry& geometry) {
    const auto bytes = render::write_avmesh(geometry);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }
    std::ofstream file{path, std::ios::binary};
    if (!file) {
        return false;
    }
    file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return static_cast<bool>(file);
}

render::IndexFormat MeshAsset::index_format() const noexcept {
    return mesh_ != nullptr ? mesh_->index_format() : render::IndexFormat::uint16;
}

std::span<const render::Submesh> MeshAsset::submeshes() const noexcept {
    return mesh_ != nullptr ? mesh_->submeshes() : std::span<const render::Submesh>{};
}

const render::Bounds* MeshAsset::bounds() const noexcept {
    return mesh_ != nullptr ? &mesh_->bounds() : nullptr;
}

MeshAssetLoader::MeshAssetLoader(Device& device) : device_(&device) {
    AV_ASSERT(device_ != nullptr);
}

std::shared_ptr<MeshAsset> MeshAssetLoader::make_asset(const render::MeshGeometry& geometry) {
    auto mesh = render::Mesh::create(*device_, geometry);
    if (!mesh) {
        return nullptr;
    }

    auto asset = std::make_shared<MeshAsset>();
    asset->vertex_count_ = mesh->vertex_count();
    asset->index_count_ = mesh->index_count();
    asset->mesh_ = std::move(mesh);
    return asset;
}

std::shared_ptr<Asset> MeshAssetLoader::load(
    [[maybe_unused]] AssetId id, std::string_view path, [[maybe_unused]] const AssetMetadata& metadata) {
    if (path.ends_with(".avmesh")) {
        const auto geometry = load_avmesh(std::filesystem::path{path});
        if (!geometry) {
            return nullptr;
        }
        return make_asset(*geometry);
    }

    if (!path.ends_with(".obj") && !path.ends_with(".mesh")) {
        return nullptr;
    }

    MeshVertexData data = load_obj(path);
    if (data.vertices.empty()) {
        return nullptr;
    }

    std::vector<std::uint16_t> indices16;
    indices16.reserve(data.indices.size());
    for (auto idx : data.indices) {
        indices16.push_back(static_cast<std::uint16_t>(idx));
    }

    return make_asset(render::mesh_geometry_from_vertices(data.vertices, indices16));
}

void MeshAssetLoader::unload(Asset* asset) {
    if (auto* mesh_asset = dynamic_cast<MeshAsset*>(asset)) {
        mesh_asset->mesh_.reset();
    }
}

bool MeshAssetLoader::reload([[maybe_unused]] Asset* asset) {
    return false;
}

MeshVertexData MeshAssetLoader::load_obj(std::string_view path) {
    MeshVertexData result;
    
    std::ifstream file{std::string{path}};
    if (!file.is_open()) {
        return result;
    }
    
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texcoords;
    
    // Vertex cache for indexed rendering
    struct VertexKey {
        int pos_idx, norm_idx, tex_idx;
        bool operator==(const VertexKey& other) const {
            return pos_idx == other.pos_idx && 
                   norm_idx == other.norm_idx && 
                   tex_idx == other.tex_idx;
        }
    };
    
    struct VertexKeyHash {
        std::size_t operator()(const VertexKey& k) const {
            return std::hash<int>{}(k.pos_idx) ^ 
                   (std::hash<int>{}(k.norm_idx) << 1) ^ 
                   (std::hash<int>{}(k.tex_idx) << 2);
        }
    };
    
    std::unordered_map<VertexKey, std::uint32_t, VertexKeyHash> vertex_cache;
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;
        
        if (type == "v") {
            // Vertex position
            float x, y, z;
            iss >> x >> y >> z;
            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        } else if (type == "vn") {
            // Vertex normal
            float x, y, z;
            iss >> x >> y >> z;
            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);
        } else if (type == "vt") {
            // Texture coordinate
            float u, v;
            iss >> u >> v;
            texcoords.push_back(u);
            texcoords.push_back(1.0f - v);  // Flip V coordinate
        } else if (type == "f") {
            // Face (supports v, v/vt, v/vt/vn, v//vn formats)
            std::string vertex_str;
            std::vector<std::uint32_t> face_indices;
            
            while (iss >> vertex_str) {
                VertexKey key{-1, -1, -1};
                
                // Parse vertex indices (format: pos/tex/norm)
                std::istringstream vss(vertex_str);
                std::string index_str;
                int component = 0;
                
                while (std::getline(vss, index_str, '/')) {
                    if (!index_str.empty()) {
                        int idx = std::stoi(index_str);
                        // OBJ uses 1-based indexing
                        if (component == 0) key.pos_idx = idx - 1;
                        else if (component == 1) key.tex_idx = idx - 1;
                        else if (component == 2) key.norm_idx = idx - 1;
                    }
                    ++component;
                }
                
                // Check if we've seen this vertex before
                auto it = vertex_cache.find(key);
                if (it != vertex_cache.end()) {
                    face_indices.push_back(it->second);
                } else {
                    // Create new vertex
                    render::Vertex vertex{};
                    
                    // Position
                    if (key.pos_idx >= 0 && key.pos_idx * 3 + 2 < positions.size()) {
                        vertex.position[0] = positions[key.pos_idx * 3 + 0];
                        vertex.position[1] = positions[key.pos_idx * 3 + 1];
                        vertex.position[2] = positions[key.pos_idx * 3 + 2];
                    }
                    
                    // Normal
                    if (key.norm_idx >= 0 && key.norm_idx * 3 + 2 < normals.size()) {
                        vertex.normal[0] = normals[key.norm_idx * 3 + 0];
                        vertex.normal[1] = normals[key.norm_idx * 3 + 1];
                        vertex.normal[2] = normals[key.norm_idx * 3 + 2];
                    }
                    
                    // Texture coordinate
                    if (key.tex_idx >= 0 && key.tex_idx * 2 + 1 < texcoords.size()) {
                        vertex.texcoord[0] = texcoords[key.tex_idx * 2 + 0];
                        vertex.texcoord[1] = texcoords[key.tex_idx * 2 + 1];
                    }
                    
                    // Default white color
                    vertex.color[0] = vertex.color[1] = vertex.color[2] = vertex.color[3] = 1.0f;
                    
                    std::uint32_t new_index = static_cast<std::uint32_t>(result.vertices.size());
                    result.vertices.push_back(vertex);
                    vertex_cache[key] = new_index;
                    face_indices.push_back(new_index);
                }
            }
            
            // Triangulate face (simple fan triangulation)
            for (std::size_t i = 1; i + 1 < face_indices.size(); ++i) {
                result.indices.push_back(face_indices[0]);
                result.indices.push_back(face_indices[i]);
                result.indices.push_back(face_indices[i + 1]);
            }
        }
    }
    
    return result;
}

} // namespace avernal
