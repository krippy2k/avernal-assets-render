#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace avernal {

inline constexpr std::uint32_t avmat_version = 1;
inline constexpr std::array<char, 4> avmat_magic{'A', 'V', 'M', 'T'};
inline constexpr std::uint32_t avmat_flag_use_texture = 1u << 0;
inline constexpr std::uint32_t avmat_flag_use_3d = 1u << 1;
inline constexpr std::uint32_t avmat_flag_use_depth = 1u << 2;
inline constexpr std::uint32_t avmat_flag_two_sided = 1u << 3;

#if defined(_MSC_VER)
#pragma pack(push, 1)
#endif

struct AvmatHeader {
    char magic[4];
    std::uint32_t version;
    std::uint64_t asset_id;
    float color[4];
    std::uint32_t flags;
    std::uint64_t texture_asset_id;
    std::uint32_t texture_path_length;
    std::uint32_t reserved;
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
;

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

static_assert(sizeof(AvmatHeader) == 52);

struct MaterialDocument {
    std::uint64_t asset_id{};
    float color[4]{1.0f, 1.0f, 1.0f, 1.0f};
    std::uint32_t flags{avmat_flag_use_3d | avmat_flag_use_depth};
    std::uint64_t texture_asset_id{};
    std::string texture_path{};
};

[[nodiscard]] std::vector<std::byte> write_avmat(const MaterialDocument& document);
[[nodiscard]] std::optional<MaterialDocument> read_avmat(std::span<const std::byte> bytes);
[[nodiscard]] bool save_avmat(const std::filesystem::path& path, const MaterialDocument& document);
[[nodiscard]] std::optional<MaterialDocument> load_avmat(const std::filesystem::path& path);

}  // namespace avernal
