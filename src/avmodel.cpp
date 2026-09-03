#include <avernal/assets_render/avmodel.hpp>

#include <cstring>
#include <fstream>

namespace avernal {
namespace {

[[nodiscard]] bool has_magic(const AvmodelFileHeader& header) noexcept {
    return header.magic[0] == avmodel_magic[0] && header.magic[1] == avmodel_magic[1] &&
           header.magic[2] == avmodel_magic[2] && header.magic[3] == avmodel_magic[3];
}

[[nodiscard]] std::uint32_t intern(std::vector<std::string>& strings, const std::string& text) {
    for (std::uint32_t i = 0; i < strings.size(); ++i) {
        if (strings[i] == text) {
            return i;
        }
    }
    strings.push_back(text);
    return static_cast<std::uint32_t>(strings.size() - 1);
}

[[nodiscard]] std::vector<std::byte> write_strings(const std::vector<std::string>& strings) {
    std::vector<std::byte> bytes;
    const auto count = static_cast<std::uint32_t>(strings.size());
    bytes.resize(8);
    std::memcpy(bytes.data(), &count, 4);
    for (const auto& value : strings) {
        const auto length = static_cast<std::uint32_t>(value.size());
        const auto offset = bytes.size();
        bytes.resize(offset + 4 + value.size());
        std::memcpy(bytes.data() + offset, &length, 4);
        if (!value.empty()) {
            std::memcpy(bytes.data() + offset + 4, value.data(), value.size());
        }
    }
    return bytes;
}

[[nodiscard]] std::optional<std::vector<std::string>> read_strings(std::span<const std::byte> bytes) {
    if (bytes.size() < 8) {
        return std::nullopt;
    }
    std::uint32_t count = 0;
    std::memcpy(&count, bytes.data(), 4);
    std::size_t offset = 8;
    std::vector<std::string> strings;
    strings.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        if (offset + 4 > bytes.size()) {
            return std::nullopt;
        }
        std::uint32_t length = 0;
        std::memcpy(&length, bytes.data() + offset, 4);
        offset += 4;
        if (offset + length > bytes.size()) {
            return std::nullopt;
        }
        strings.emplace_back(reinterpret_cast<const char*>(bytes.data() + offset), length);
        offset += length;
    }
    return strings;
}

[[nodiscard]] std::optional<std::vector<std::byte>> read_file(const std::filesystem::path& path) {
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
    return bytes;
}

}  // namespace

std::vector<std::byte> write_avmodel(const ModelDocument& document) {
    std::vector<std::string> strings;
    std::vector<AvmodelNode> nodes(document.nodes.size());
    for (std::size_t i = 0; i < document.nodes.size(); ++i) {
        const auto& src = document.nodes[i];
        auto& dst = nodes[i];
        dst.parent = src.parent;
        dst.name_index = src.name.empty() ? avmodel_none : intern(strings, src.name);
        std::memcpy(dst.position, src.position, sizeof(dst.position));
        std::memcpy(dst.rotation, src.rotation, sizeof(dst.rotation));
        std::memcpy(dst.scale, src.scale, sizeof(dst.scale));
    }

    std::vector<AvmodelPart> parts(document.parts.size());
    for (std::size_t i = 0; i < document.parts.size(); ++i) {
        parts[i].node = document.parts[i].node;
        parts[i].mesh = document.parts[i].mesh;
        parts[i].material = document.parts[i].material;
        parts[i].reserved = 0;
    }

    std::vector<AvmodelAssetRef> meshes(document.meshes.size());
    for (std::size_t i = 0; i < document.meshes.size(); ++i) {
        meshes[i].asset_id = document.meshes[i].asset_id;
        meshes[i].path_index = intern(strings, document.meshes[i].path);
        meshes[i].reserved = 0;
    }

    std::vector<AvmodelAssetRef> materials(document.materials.size());
    for (std::size_t i = 0; i < document.materials.size(); ++i) {
        materials[i].asset_id = document.materials[i].asset_id;
        materials[i].path_index = intern(strings, document.materials[i].path);
        materials[i].reserved = 0;
    }

    const auto string_bytes = write_strings(strings);

    AvmodelFileHeader header{};
    std::memcpy(header.magic, avmodel_magic.data(), 4);
    header.version = avmodel_version;
    header.asset_id = document.asset_id;
    header.node_count = static_cast<std::uint32_t>(nodes.size());
    header.part_count = static_cast<std::uint32_t>(parts.size());
    header.mesh_count = static_cast<std::uint32_t>(meshes.size());
    header.material_count = static_cast<std::uint32_t>(materials.size());
    header.reserved = 0;

    std::vector<std::byte> bytes(sizeof(AvmodelFileHeader) + nodes.size() * sizeof(AvmodelNode) +
        parts.size() * sizeof(AvmodelPart) + meshes.size() * sizeof(AvmodelAssetRef) +
        materials.size() * sizeof(AvmodelAssetRef) + string_bytes.size());

    std::size_t offset = 0;
    const auto append = [&](const void* data, std::size_t size) {
        if (size == 0) {
            return;
        }
        std::memcpy(bytes.data() + offset, data, size);
        offset += size;
    };

    append(&header, sizeof(header));
    append(nodes.data(), nodes.size() * sizeof(AvmodelNode));
    append(parts.data(), parts.size() * sizeof(AvmodelPart));
    append(meshes.data(), meshes.size() * sizeof(AvmodelAssetRef));
    append(materials.data(), materials.size() * sizeof(AvmodelAssetRef));
    append(string_bytes.data(), string_bytes.size());
    bytes.resize(offset);
    return bytes;
}

std::optional<ModelDocument> read_avmodel(std::span<const std::byte> bytes) {
    if (bytes.size() < sizeof(AvmodelFileHeader)) {
        return std::nullopt;
    }

    AvmodelFileHeader header{};
    std::memcpy(&header, bytes.data(), sizeof(header));
    if (!has_magic(header) || header.version != avmodel_version) {
        return std::nullopt;
    }

    std::size_t offset = sizeof(AvmodelFileHeader);
    const auto take = [&](std::size_t size) -> std::optional<std::span<const std::byte>> {
        if (offset + size > bytes.size()) {
            return std::nullopt;
        }
        const auto slice = bytes.subspan(offset, size);
        offset += size;
        return slice;
    };

    const auto node_bytes = take(header.node_count * sizeof(AvmodelNode));
    const auto part_bytes = take(header.part_count * sizeof(AvmodelPart));
    const auto mesh_bytes = take(header.mesh_count * sizeof(AvmodelAssetRef));
    const auto material_bytes = take(header.material_count * sizeof(AvmodelAssetRef));
    if (!node_bytes || !part_bytes || !mesh_bytes || !material_bytes) {
        return std::nullopt;
    }
    const auto strings = read_strings(bytes.subspan(offset));
    if (!strings) {
        return std::nullopt;
    }

    ModelDocument document{};
    document.asset_id = header.asset_id;
    document.nodes.resize(header.node_count);
    for (std::uint32_t i = 0; i < header.node_count; ++i) {
        AvmodelNode packed{};
        std::memcpy(&packed, node_bytes->data() + i * sizeof(AvmodelNode), sizeof(packed));
        auto& node = document.nodes[i];
        node.parent = packed.parent;
        if (packed.name_index != avmodel_none) {
            if (packed.name_index >= strings->size()) {
                return std::nullopt;
            }
            node.name = (*strings)[packed.name_index];
        }
        std::memcpy(node.position, packed.position, sizeof(node.position));
        std::memcpy(node.rotation, packed.rotation, sizeof(node.rotation));
        std::memcpy(node.scale, packed.scale, sizeof(node.scale));
    }

    document.parts.resize(header.part_count);
    for (std::uint32_t i = 0; i < header.part_count; ++i) {
        AvmodelPart packed{};
        std::memcpy(&packed, part_bytes->data() + i * sizeof(AvmodelPart), sizeof(packed));
        document.parts[i] = {.node = packed.node, .mesh = packed.mesh, .material = packed.material};
    }

    const auto read_refs = [&](std::span<const std::byte> packed_bytes, std::uint32_t count,
                               std::vector<ModelAssetRef>& out) -> bool {
        out.resize(count);
        for (std::uint32_t i = 0; i < count; ++i) {
            AvmodelAssetRef packed{};
            std::memcpy(&packed, packed_bytes.data() + i * sizeof(AvmodelAssetRef), sizeof(packed));
            if (packed.path_index >= strings->size()) {
                return false;
            }
            out[i] = {.asset_id = packed.asset_id, .path = (*strings)[packed.path_index]};
        }
        return true;
    };

    if (!read_refs(*mesh_bytes, header.mesh_count, document.meshes) ||
        !read_refs(*material_bytes, header.material_count, document.materials)) {
        return std::nullopt;
    }
    return document;
}

bool save_avmodel(const std::filesystem::path& path, const ModelDocument& document) {
    const auto bytes = write_avmodel(document);
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

std::optional<ModelDocument> load_avmodel(const std::filesystem::path& path) {
    const auto bytes = read_file(path);
    if (!bytes) {
        return std::nullopt;
    }
    return read_avmodel(*bytes);
}

}  // namespace avernal
