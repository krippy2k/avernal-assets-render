#include <avernal/assets_render/avmat.hpp>
#include <avernal/assets_render/avmodel.hpp>
#include <avernal/assets_render/avtex.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

namespace {

[[nodiscard]] std::filesystem::path temp_file(const char* name) {
    return std::filesystem::temp_directory_path() / name;
}

}  // namespace

TEST(AvtexIo, RoundtripsFile) {
    const auto path = temp_file("avernal-test.avtex");
    avernal::TextureImage original{};
    original.width = 2;
    original.height = 1;
    original.pixels = {255, 0, 0, 255, 0, 255, 0, 255};

    ASSERT_TRUE(avernal::save_avtex(path, original));
    const auto loaded = avernal::load_avtex(path);
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->width, 2u);
    EXPECT_EQ(loaded->height, 1u);
    EXPECT_EQ(loaded->pixels, original.pixels);
    std::filesystem::remove(path);
}

TEST(AvtexIo, TruncatedFileReturnsNull) {
    const auto path = temp_file("avernal-truncated.avtex");
    {
        std::ofstream file{path, std::ios::binary};
        file.write("AVTX", 4);
    }
    EXPECT_FALSE(avernal::load_avtex(path).has_value());
    std::filesystem::remove(path);
}

TEST(AvmatIo, RoundtripsFile) {
    const auto path = temp_file("avernal-test.avmat");
    avernal::MaterialDocument original{};
    original.asset_id = 42;
    original.color[0] = 0.25f;
    original.color[1] = 0.5f;
    original.color[2] = 0.75f;
    original.color[3] = 1.0f;
    original.flags = avernal::avmat_flag_use_texture | avernal::avmat_flag_use_3d |
                     avernal::avmat_flag_use_depth;
    original.texture_asset_id = 7;
    original.texture_path = "../textures/albedo.avtex";

    ASSERT_TRUE(avernal::save_avmat(path, original));
    const auto loaded = avernal::load_avmat(path);
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->asset_id, original.asset_id);
    EXPECT_EQ(loaded->flags, original.flags);
    EXPECT_EQ(loaded->texture_asset_id, original.texture_asset_id);
    EXPECT_EQ(loaded->texture_path, original.texture_path);
    EXPECT_FLOAT_EQ(loaded->color[0], original.color[0]);
    EXPECT_FLOAT_EQ(loaded->color[1], original.color[1]);
    EXPECT_FLOAT_EQ(loaded->color[2], original.color[2]);
    EXPECT_FLOAT_EQ(loaded->color[3], original.color[3]);
    std::filesystem::remove(path);
}

TEST(AvmodelIo, RoundtripsFile) {
    const auto path = temp_file("avernal-test.avmodel");
    avernal::ModelDocument original{};
    original.asset_id = 99;
    original.nodes.push_back(avernal::ModelNode{
        .parent = avernal::avmodel_none,
        .name = "root",
        .position = {1.0f, 2.0f, 3.0f},
        .rotation = {0.0f, 0.0f, 0.0f, 1.0f},
        .scale = {1.0f, 1.0f, 1.0f},
    });
    original.parts.push_back(avernal::ModelPart{.node = 0, .mesh = 0, .material = 0});
    original.meshes.push_back(avernal::ModelAssetRef{.asset_id = 1, .path = "meshes/tri.avmesh"});
    original.materials.push_back(
        avernal::ModelAssetRef{.asset_id = 2, .path = "materials/red.avmat"});

    ASSERT_TRUE(avernal::save_avmodel(path, original));
    const auto loaded = avernal::load_avmodel(path);
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->asset_id, original.asset_id);
    ASSERT_EQ(loaded->nodes.size(), 1u);
    EXPECT_EQ(loaded->nodes[0].name, "root");
    EXPECT_FLOAT_EQ(loaded->nodes[0].position[2], 3.0f);
    ASSERT_EQ(loaded->parts.size(), 1u);
    EXPECT_EQ(loaded->parts[0].mesh, 0u);
    ASSERT_EQ(loaded->meshes.size(), 1u);
    EXPECT_EQ(loaded->meshes[0].path, "meshes/tri.avmesh");
    ASSERT_EQ(loaded->materials.size(), 1u);
    EXPECT_EQ(loaded->materials[0].path, "materials/red.avmat");
    std::filesystem::remove(path);
}
