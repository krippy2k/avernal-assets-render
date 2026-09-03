#include <avernal/assets_render/image.hpp>

#include "stb_image.h"

namespace avernal {

std::optional<ImageRgba8> load_image_rgba8(const std::filesystem::path& path) {
    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == nullptr || width <= 0 || height <= 0) {
        return std::nullopt;
    }

    ImageRgba8 image{};
    image.width = static_cast<std::uint32_t>(width);
    image.height = static_cast<std::uint32_t>(height);
    image.pixels.assign(pixels, pixels + static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4u);
    stbi_image_free(pixels);
    return image;
}

std::optional<ImageRgba8> load_image_rgba8_from_memory(std::span<const std::byte> bytes) {
    if (bytes.empty()) {
        return std::nullopt;
    }

    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc* pixels = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(bytes.data()),
        static_cast<int>(bytes.size()), &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == nullptr || width <= 0 || height <= 0) {
        return std::nullopt;
    }

    ImageRgba8 image{};
    image.width = static_cast<std::uint32_t>(width);
    image.height = static_cast<std::uint32_t>(height);
    image.pixels.assign(pixels, pixels + static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4u);
    stbi_image_free(pixels);
    return image;
}

}  // namespace avernal
