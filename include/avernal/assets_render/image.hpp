#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>

namespace avernal {

struct ImageRgba8 {
    std::uint32_t width{};
    std::uint32_t height{};
    std::vector<std::uint8_t> pixels{};
};

[[nodiscard]] std::optional<ImageRgba8> load_image_rgba8(const std::filesystem::path& path);
[[nodiscard]] std::optional<ImageRgba8> load_image_rgba8_from_memory(std::span<const std::byte> bytes);

}  // namespace avernal
