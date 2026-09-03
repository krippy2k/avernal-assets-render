cmake_minimum_required(VERSION 3.28)

project(
    avernal-assets-render
    VERSION 0.1.0
    DESCRIPTION "Rendering asset loaders for the Avernal game engine"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

include(FetchContent)

# Find dependencies
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../avernal-core/CMakeLists.txt")
    set(AVERNAL_CORE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../avernal-core")
else()
    set(AVERNAL_CORE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../avernal-core")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../avernal-assets/CMakeLists.txt")
    set(AVERNAL_ASSETS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../avernal-assets")
else()
    set(AVERNAL_ASSETS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../avernal-assets")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../avernal-render/CMakeLists.txt")
    set(AVERNAL_RENDER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../avernal-render")
else()
    set(AVERNAL_RENDER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../avernal-render")
endif()

if(NOT EXISTS "${AVERNAL_CORE_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "avernal-core not found at:\n"
        "  ${AVERNAL_CORE_SOURCE_DIR}\n"
        "Place avernal-core as a sibling of this project.")
endif()

if(NOT EXISTS "${AVERNAL_ASSETS_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "avernal-assets not found at:\n"
        "  ${AVERNAL_ASSETS_SOURCE_DIR}\n"
        "Place avernal-assets as a sibling of this project.")
endif()

if(NOT EXISTS "${AVERNAL_RENDER_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "avernal-render not found at:\n"
        "  ${AVERNAL_RENDER_SOURCE_DIR}\n"
        "Place avernal-render as a sibling of this project.")
endif()

FetchContent_Declare(
    avernal_core
    SOURCE_DIR "${AVERNAL_CORE_SOURCE_DIR}"
    EXCLUDE_FROM_ALL
)

FetchContent_Declare(
    avernal_assets
    SOURCE_DIR "${AVERNAL_ASSETS_SOURCE_DIR}"
    EXCLUDE_FROM_ALL
)

FetchContent_Declare(
    avernal_render
    SOURCE_DIR "${AVERNAL_RENDER_SOURCE_DIR}"
    EXCLUDE_FROM_ALL
)

set(AVERNAL_CORE_BUILD_DEMO OFF)

FetchContent_MakeAvailable(avernal_core avernal_assets avernal_render)

add_library(avernal-assets-render)
add_library(avernal::assets_render ALIAS avernal-assets-render)

target_sources(
    avernal-assets-render
    PRIVATE
        src/avmat.cpp
        src/avmodel.cpp
        src/avtex.cpp
        src/image.cpp
        src/material_asset.cpp
        src/mesh_asset.cpp
        src/model_asset.cpp
        src/shader_asset.cpp
        src/stb_image_impl.cpp
        src/texture_asset.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/avernal/assets_render/assets_render.hpp
            include/avernal/assets_render/avmat.hpp
            include/avernal/assets_render/avmodel.hpp
            include/avernal/assets_render/avtex.hpp
            include/avernal/assets_render/image.hpp
            include/avernal/assets_render/material_asset.hpp
            include/avernal/assets_render/mesh_asset.hpp
            include/avernal/assets_render/model_asset.hpp
            include/avernal/assets_render/shader_asset.hpp
            include/avernal/assets_render/texture_asset.hpp
)

target_include_directories(avernal-assets-render PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

target_link_libraries(avernal-assets-render PUBLIC 
    avernal::core 
    avernal::assets
    avernal::render
)

target_compile_features(avernal-assets-render PUBLIC cxx_std_23)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(avernal-assets-render PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(avernal-assets-render PRIVATE /W4)
endif()

if(PROJECT_IS_TOP_LEVEL)
    option(AVERNAL_ASSETS_RENDER_BUILD_TESTS "Build Avernal Assets Render unit tests" ON)
else()
    option(AVERNAL_ASSETS_RENDER_BUILD_TESTS "Build Avernal Assets Render unit tests" OFF)
endif()

if(AVERNAL_ASSETS_RENDER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
