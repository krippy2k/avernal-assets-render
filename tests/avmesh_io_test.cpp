#include <avernal/assets_render/mesh_asset.hpp>
#include <avernal/render/mesh.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

namespace {

[[nodiscard]] avernal::render::MeshGeometry make_triangle() {
    using namespace avernal::render;
    const Vertex vertices[] = {
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    };
    const std::uint16_t indices[] = {0, 1, 2};
    return mesh_geometry_from_vertices(vertices, indices);
}

}  // namespace

TEST(AvmeshIo, RoundtripsFile) {
    const auto path = std::filesystem::temp_directory_path() / "avernal-test.avmesh";
    const auto original = make_triangle();
    ASSERT_TRUE(avernal::save_avmesh(path, original));

    const auto loaded = avernal::load_avmesh(path);
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->streams[0].vertex_count, 3u);
    EXPECT_EQ(loaded->index_data, original.index_data);
    EXPECT_EQ(loaded->stream_data[0], original.stream_data[0]);
    EXPECT_EQ(loaded->submeshes[0].index_count, 3u);

    std::filesystem::remove(path);
}

TEST(AvmeshIo, MissingFileReturnsNull) {
    EXPECT_FALSE(avernal::load_avmesh("no-such-file.avmesh").has_value());
}

TEST(AvmeshIo, TruncatedFileReturnsNull) {
    const auto path = std::filesystem::temp_directory_path() / "avernal-truncated.avmesh";
    {
        std::ofstream file{path, std::ios::binary};
        file.write("AVMS", 4);
    }
    EXPECT_FALSE(avernal::load_avmesh(path).has_value());
    std::filesystem::remove(path);
}
