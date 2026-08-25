# avernal-render-assets

Rendering-specific asset loaders for the Avernal game engine.

## Overview

This library provides concrete implementations of asset loaders for rendering-related assets:
- **Textures**: PNG, JPG, BMP, TGA formats via stb_image
- **Materials**: Custom `.mat` format with texture references
- **Meshes**: OBJ format support with vertex deduplication
- **Shaders**: HLSL, GLSL, and custom multi-stage `.shader` format

## Features

### Texture Loading

- Automatic format detection
- RGBA conversion
- Hot-reloading support
- Integration with `avernal-render::Texture`

```cpp
auto loader = std::make_unique<TextureAssetLoader>(device);
manager.register_loader(std::move(loader));

auto texture = manager.load<TextureAsset>("textures/wall.png");
if (texture) {
    render::Texture* render_tex = texture->texture();
    uint32_t width = texture->width();
    uint32_t height = texture->height();
}
```

### Material Loading

- Texture references
- Color properties
- 3D/2D rendering modes
- Dependency management

**Material File Format** (`.mat`):
```
texture: assets/textures/wall.png
color: 1.0 1.0 1.0 1.0
use_3d: true
```

```cpp
auto loader = std::make_unique<MaterialAssetLoader>(device, manager);
manager.register_loader(std::move(loader));

auto material = manager.load<MaterialAsset>("materials/brick.mat");
if (material) {
    render::Material* mat = material->material();
    auto tex = material->texture();  // Associated texture
}
```

### Mesh Loading

- OBJ file format
- Vertex deduplication
- Automatic normal loading
- Texture coordinate support

```cpp
auto loader = std::make_unique<MeshAssetLoader>(device);
manager.register_loader(std::move(loader));

auto mesh = manager.load<MeshAsset>("models/cube.obj");
if (mesh) {
    render::Mesh* render_mesh = mesh->mesh();
    uint32_t vertex_count = mesh->vertex_count();
    uint32_t index_count = mesh->index_count();
}
```

### Shader Loading

- Multi-stage shader files
- Per-stage files (.vert, .frag, .comp)
- Hot-reloading support

**Custom Shader Format** (`.shader`):
```glsl
#shader vertex
#version 450
layout(location = 0) in vec3 position;
// ...

#shader fragment
#version 450
layout(location = 0) out vec4 color;
// ...
```

```cpp
auto loader = std::make_unique<ShaderAssetLoader>();
manager.register_loader(std::move(loader));

auto shader = manager.load<ShaderAsset>("shaders/basic.shader");
if (shader) {
    if (shader->has_vertex()) {
        const std::string& vs = shader->source().vertex_code;
    }
    if (shader->has_fragment()) {
        const std::string& fs = shader->source().fragment_code;
    }
}
```

## Integration Example

```cpp
#include <avernal/assets/assets.hpp>
#include <avernal/render_assets/render_assets.hpp>

int main() {
    // Create RHI device
    auto device = avernal::create_d3d12_device({});
    
    // Create asset manager
    avernal::AssetManager manager;
    
    // Register all rendering loaders
    manager.register_loader(
        std::make_unique<avernal::TextureAssetLoader>(*device)
    );
    manager.register_loader(
        std::make_unique<avernal::MaterialAssetLoader>(*device, manager)
    );
    manager.register_loader(
        std::make_unique<avernal::MeshAssetLoader>(*device)
    );
    manager.register_loader(
        std::make_unique<avernal::ShaderAssetLoader>()
    );
    
    // Scan for assets
    manager.registry().scan_directory("assets/");
    
    // Load assets
    auto texture = manager.load<avernal::TextureAsset>("assets/textures/wall.png");
    auto mesh = manager.load<avernal::MeshAsset>("assets/models/cube.obj");
    auto material = manager.load<avernal::MaterialAsset>("assets/materials/default.mat");
    
    return 0;
}
```

## Dependencies

- `avernal-core` - Core utilities
- `avernal-assets` - Generic asset system
- `avernal-render` - Rendering abstractions
- `stb_image` - Image loading (header-only)

## Supported Formats

### Textures
- PNG
- JPG/JPEG
- BMP
- TGA

### Meshes
- OBJ (with normals and texture coordinates)
- Custom `.mesh` format (future)

### Materials
- `.mat` - Custom material definition
- `.material` - Alternative extension

### Shaders
- `.shader` - Multi-stage custom format
- `.vert` - Vertex shader
- `.frag` - Fragment shader
- `.comp` - Compute shader
- `.hlsl` - HLSL source
- `.glsl` - GLSL source

## Build

```bash
cmake -B build -S .
cmake --build build
```

This library requires C++23 and depends on `avernal-core`, `avernal-assets`, and `avernal-render`.
