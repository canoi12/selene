# Selene shader plugin

Compiles the **canonical GLSL sources** in each shader folder into per-backend
binaries and emits Lua modules the engine loads at runtime.

```
plugins/Shaders/
  build_shaders.py
  runtime.lua
  init.lua
  sprite2d/
    vertex.vs
    fragment.fs
    bin.lua            generated
    meta.lua           generated
    init.lua           generated
  _work/               intermediates, not loaded at runtime
```

## Why a canonical source

Every effect is written **once** in GLSL with explicit `layout(location = …)` and
`layout(binding = …)`. The tool derives the other backends, including SPIR-V
reflection metadata that is the source of truth for the engine's vertex layout
and resource bindings.

## Pipeline (per shader)

```
<name>/vertex.vs / <name>/fragment.fs     (canonical GLSL, #version 450)
        |  glslangValidator -V --target-env vulkan1.0
        v
   <name>.vertex.spv / <name>.fragment.spv           (SPIR-V: Vulkan, D3D12)
        |
        |-- spirv-cross --reflect   --> JSON --> meta.lua
        |-- spirv-cross --hlsl      --> HLSL      (only for --backend d3d11)
        `-- spirv-cross --es --version 310 --> GLSL ES (#version 310 es)
```

The binaries are embedded as **base85** strings in `bin.lua`. Reflection metadata
goes to `meta.lua`. `init.lua` returns `get(renderer, stage)`.

## Requirements

On `PATH` **or** under `$VULKAN_SDK/Bin` (Windows: `%VULKAN_SDK%\Bin`):

- `glslangValidator` — GLSL → SPIR-V
- `spirv-cross` — SPIR-V → HLSL / GLSL ES, and `--reflect` JSON

Both ship with the Vulkan SDK. Python 3.8+ is required; **no pip packages** —
only the standard library.

## Usage

```sh
# list discovered shaders (folders with vertex.vs / fragment.fs)
python build_shaders.py --list

# build ALL backend variants (default) into each shader folder
python build_shaders.py

# build only some shaders, add preprocessor defines
python build_shaders.py --shaders sprite2d --defines DEBUG,USE_SHADOWS=1

# build only ONE backend variant (e.g. just Vulkan SPIR-V)
python build_shaders.py --backend vulkan

# override tool locations if not on PATH / not under VULKAN_SDK
python build_shaders.py --glsllang /path/to/glslangValidator --spirv_cross /path/to/spirv-cross
```

`--backend all` (default) emits every variant: `vulkan` (SPIR-V, base85),
`d3d11` (HLSL text), `opengl` (canonical GLSL text), `opengles` (GLSL ES text).

Outputs (default `--src` and `--out` are this directory):

- `plugins/Shaders/<name>/bin.lua` — backend blobs, keyed by `vertex` / `pixel`.
- `plugins/Shaders/<name>/meta.lua` — SPIR-V reflection for the same stages.
- `plugins/Shaders/<name>/init.lua` — `get(renderer, stage)` for that shader.
- `plugins/Shaders/_work/` — intermediates (`.spv`, `.hlsl`, `.gles.glsl`).
  Safe to delete; not loaded at runtime.

## Output format

Stages are keyed by the names `create_shader` accepts (`vertex`, `pixel`):

```lua
-- bin.lua
return {
    vertex = {
        vulkan   = { enc = "base85", data = "<SPIR-V as base85>" },
        d3d11    = { enc = "text",   data = "<HLSL source>" },
        opengles = { enc = "text",   data = "<GLSL ES source>" },
        opengl   = { enc = "text",   data = "<desktop GLSL source>" },
    },
    pixel = { ... },
}

-- meta.lua
return {
    vertex = {
        inputs  = { { location = 0, name = "a_position", type = "float", glsl_type = "vec3", size = 3 }, ... },
        outputs = { { location = 0, name = "v_color", type = "float", glsl_type = "vec4", size = 4 }, ... },
        ubos    = { { set = 0, binding = 0, name = "Matrices", block_size = 64,
                      members = { { name = "world", type = "float", glsl_type = "mat4", offset = 0, size = 4 } } } },
        ssbos = {}, samplers = {}, images = {},
    },
    pixel = { ... },
}
```

Notes:

- `enc = "base85"` means `data` is RFC 1924 base85 (Python `base64.b85encode`);
  `plugins/Shaders/runtime.lua` decodes it to raw SPIR-V bytes.
  `enc = "text"` means the source is used as-is.
- `type` is the engine's scalar type name (matches `type_name_options`:
  `byte, ubyte, short, ushort, int, uint, float, double`); `size` is the
  component count (e.g. `vec3` → `type="float", size=3`).
- `glsl_type` keeps the original GLSL type string for reference.

## Consuming the data

The plugin does **not** branch on backend. `require('plugins.Shaders')` loads
the right variant based on `renderer:get_info().backend` (and `.es` for OpenGL
ES), decodes base85 SPIR-V when needed, and calls `renderer:create_shader`:

```lua
local shaders = require('plugins.Shaders')
local vs = shaders.get(renderer, 'sprite2d', 'vertex')  -- stage: 'vertex' | 'pixel'
```

`shaders.meta(name, stage)` returns the reflection metadata.

Each folder's `init.lua` returns that same `get` already bound to the shader:

```lua
local sprite2d = require('plugins.Shaders.sprite2d')
local vs = sprite2d(renderer, 'vertex')
```

## Adding a shader

1. Create `plugins/Shaders/myname/` with `vertex.vs` and `fragment.fs`
   (`#version 450`, explicit locations/bindings).
2. `python build_shaders.py` (regenerates `bin.lua`, `meta.lua`, `init.lua`).
3. `shaders.get(renderer, 'myname', 'vertex'|'pixel')`.

## Known caveat for D3D11

`spirv-cross --hlsl` assigns its own input semantics (`TEXCOORD0/1/2`) instead of
the source variable names. The engine's D3D11 input layout currently uses the
attribute name as `SemanticName`, so a later pass will use the reflected metadata
(`inputs[].location`) to build a deterministic `location → semantic` mapping
(e.g. `TEXCOORD{location}`) so the HLSL and the input layout always agree. The
HLSL entry point is `main`; combined samplers map to `t{n}`/`s{n}` registers via
`binding`.
