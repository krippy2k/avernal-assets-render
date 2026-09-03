#include <avernal/assets_render/avtex.hpp>

#include <cstring>
#include <fstream>

namespace avernal {
namespace {

[[nodiscard]] bool has_magic(const AvtexHeader& header) noexcept {
    return header.magic[0] == avtex_magic[0] && header.magic[1] == avtex_magic[1] &&
           header.magic[2] == avtex_magic[2] && header.magic[3] == avtex_magic[3];
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

std::vector<std::byte> write_avtex(const TextureImage& image) {
    AvtexHeader header{};
    std::memcpy(header.magic, avtex_magic.data(), 4);
    header.version = avtex_version;
    header.width = image.width;
    header.height = image.height;
    header.format = image.format;
    header.flags = 0;
    header.pixel_bytes = image.pixels.size();

    std::vector<std::byte> bytes(sizeof(AvtexHeader) + image.pixels.size());
    std::memcpy(bytes.data(), &header, sizeof(header));
    if (!image.pixels.empty()) {
        std::memcpy(bytes.data() + sizeof(AvtexHeader), image.pixels.data(), image.pixels.size());
    }
    return bytes;
}

std::optional<TextureImage> read_avtex(std::span<const std::byte> bytes) {
    if (bytes.size() < sizeof(AvtexHeader)) {
        return std::nullopt;
    }

    AvtexHeader header{};
    std::memcpy(&header, bytes.data(), sizeof(header));
    if (!has_magic(header) || header.version != avtex_version) {
        return std::nullopt;
    }
    if (header.format != AvtexFormat::rgba8_unorm || header.width == 0 || header.height == 0) {
        return std::nullopt;
    }
    const auto expected = static_cast<std::uint64_t>(header.width) * header.height * 4u;
    if (header.pixel_bytes != expected || sizeof(AvtexHeader) + expected > bytes.size()) {
        return std::nullopt;
    }

    TextureImage image{};
    image.width = header.width;
    image.height = header.height;
    image.format = header.format;
    image.pixels.resize(static_cast<std::size_t>(expected));
    std::memcpy(image.pixels.data(), bytes.data() + sizeof(AvtexHeader), image.pixels.size());
    return image;
}

bool save_avtex(const std::filesystem::path& path, const TextureImage& image) {
    const auto bytes = write_avtex(image);
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

std::optional<TextureImage> load_avtex(const std::filesystem::path& path) {
    const auto bytes = read_file(path);
    if (!bytes) {
        return std::nullopt;
    }
    return read_avtex(*bytes);
}

}  // namespace avernal
