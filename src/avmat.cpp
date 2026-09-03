#include <avernal/assets_render/avmat.hpp>

#include <cstring>
#include <fstream>

namespace avernal {
namespace {

[[nodiscard]] bool has_magic(const AvmatHeader& header) noexcept {
    return header.magic[0] == avmat_magic[0] && header.magic[1] == avmat_magic[1] &&
           header.magic[2] == avmat_magic[2] && header.magic[3] == avmat_magic[3];
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

std::vector<std::byte> write_avmat(const MaterialDocument& document) {
    AvmatHeader header{};
    std::memcpy(header.magic, avmat_magic.data(), 4);
    header.version = avmat_version;
    header.asset_id = document.asset_id;
    header.color[0] = document.color[0];
    header.color[1] = document.color[1];
    header.color[2] = document.color[2];
    header.color[3] = document.color[3];
    header.flags = document.flags;
    header.texture_asset_id = document.texture_asset_id;
    header.texture_path_length = static_cast<std::uint32_t>(document.texture_path.size());
    header.reserved = 0;

    std::vector<std::byte> bytes(sizeof(AvmatHeader) + document.texture_path.size());
    std::memcpy(bytes.data(), &header, sizeof(header));
    if (!document.texture_path.empty()) {
        std::memcpy(bytes.data() + sizeof(AvmatHeader), document.texture_path.data(),
            document.texture_path.size());
    }
    return bytes;
}

std::optional<MaterialDocument> read_avmat(std::span<const std::byte> bytes) {
    if (bytes.size() < sizeof(AvmatHeader)) {
        return std::nullopt;
    }

    AvmatHeader header{};
    std::memcpy(&header, bytes.data(), sizeof(header));
    if (!has_magic(header) || header.version != avmat_version) {
        return std::nullopt;
    }
    if (sizeof(AvmatHeader) + header.texture_path_length > bytes.size()) {
        return std::nullopt;
    }

    MaterialDocument document{};
    document.asset_id = header.asset_id;
    document.color[0] = header.color[0];
    document.color[1] = header.color[1];
    document.color[2] = header.color[2];
    document.color[3] = header.color[3];
    document.flags = header.flags;
    document.texture_asset_id = header.texture_asset_id;
    if (header.texture_path_length > 0) {
        document.texture_path.assign(
            reinterpret_cast<const char*>(bytes.data() + sizeof(AvmatHeader)),
            header.texture_path_length);
    }
    return document;
}

bool save_avmat(const std::filesystem::path& path, const MaterialDocument& document) {
    const auto bytes = write_avmat(document);
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

std::optional<MaterialDocument> load_avmat(const std::filesystem::path& path) {
    const auto bytes = read_file(path);
    if (!bytes) {
        return std::nullopt;
    }
    return read_avmat(*bytes);
}

}  // namespace avernal
