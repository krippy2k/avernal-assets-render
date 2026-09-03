#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>

namespace avernal {

inline constexpr std::uint32_t avtex_version = 1;
inline constexpr std::array<char, 4> avtex_magic{'A', 'V', 'T', 'X'};

enum class AvtexFormat : std::uint32_t {
    rgba8_unorm = 1,
};

#if defined(_MSC_VER)
#pragma pack(push, 1)
#endif

struct AvtexHeader {
    char magic[4];
    std::uint32_t version;
    std::uint32_t width;
    std::uint32_t height;
    AvtexFormat format;
    std::uint32_t flags;
    std::uint64_t pixel_bytes;
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
;

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

static_assert(sizeof(AvtexHeader) == 32);

struct TextureImage {
    std::uint32_t width{};
    std::uint32_t height{};
    AvtexFormat format{AvtexFormat::rgba8_unorm};
    std::vector<std::uint8_t> pixels{};
};

[[nodiscard]] std::vector<std::byte> write_avtex(const TextureImage& image);
[[nodiscard]] std::optional<TextureImage> read_avtex(std::span<const std::byte> bytes);
[[nodiscard]] bool save_avtex(const std::filesystem::path& path, const TextureImage& image);
[[nodiscard]] std::optional<TextureImage> load_avtex(const std::filesystem::path& path);

}  // namespace avernal
