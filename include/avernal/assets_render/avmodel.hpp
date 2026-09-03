#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace avernal {

inline constexpr std::uint32_t avmodel_version = 1;
inline constexpr std::array<char, 4> avmodel_magic{'A', 'V', 'M', 'D'};
inline constexpr std::uint32_t avmodel_none = 0xFFFFFFFFu;

#if defined(_MSC_VER)
#pragma pack(push, 1)
#endif

struct AvmodelFileHeader {
    char magic[4];
    std::uint32_t version;
    std::uint64_t asset_id;
    std::uint32_t node_count;
    std::uint32_t part_count;
    std::uint32_t mesh_count;
    std::uint32_t material_count;
    std::uint32_t reserved;
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
;

struct AvmodelNode {
    std::uint32_t parent;
    std::uint32_t name_index;
    float position[3];
    float rotation[4];
    float scale[3];
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
;

struct AvmodelPart {
    std::uint32_t node;
    std::uint32_t mesh;
    std::uint32_t material;
    std::uint32_t reserved;
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
;

struct AvmodelAssetRef {
    std::uint64_t asset_id;
    std::uint32_t path_index;
    std::uint32_t reserved;
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
;

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

static_assert(sizeof(AvmodelFileHeader) == 36);
static_assert(sizeof(AvmodelNode) == 48);
static_assert(sizeof(AvmodelPart) == 16);
static_assert(sizeof(AvmodelAssetRef) == 16);

struct ModelNode {
    std::uint32_t parent{avmodel_none};
    std::string name{};
    float position[3]{};
    float rotation[4]{0.0f, 0.0f, 0.0f, 1.0f};
    float scale[3]{1.0f, 1.0f, 1.0f};
};

struct ModelPart {
    std::uint32_t node{avmodel_none};
    std::uint32_t mesh{avmodel_none};
    std::uint32_t material{avmodel_none};
};

struct ModelAssetRef {
    std::uint64_t asset_id{};
    std::string path{};
};

struct ModelDocument {
    std::uint64_t asset_id{};
    std::vector<ModelNode> nodes{};
    std::vector<ModelPart> parts{};
    std::vector<ModelAssetRef> meshes{};
    std::vector<ModelAssetRef> materials{};
};

[[nodiscard]] std::vector<std::byte> write_avmodel(const ModelDocument& document);
[[nodiscard]] std::optional<ModelDocument> read_avmodel(std::span<const std::byte> bytes);
[[nodiscard]] bool save_avmodel(const std::filesystem::path& path, const ModelDocument& document);
[[nodiscard]] std::optional<ModelDocument> load_avmodel(const std::filesystem::path& path);

}  // namespace avernal
