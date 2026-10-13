# Selene shader build tool

Compiles the **canonical GLSL sources** (`.vs` / `.fs`) in this folder into
per-backend binaries and emits a single Lua data module (`shaders_data.lua`)
that the engine loads.

This is **Phase 1** of the shader-pipeline refactor: it only *generates* assets.
It does not touch the engine or the plugins yet.

## Why a canonical source

Previously every effect existed three times (GLSL string in Lua, HLSL string in
Lua, and a prebuilt `.spv`). Here you write the shader **once** in GLSL with
explicit `layout(location = …)` and `layout(binding = …)`, and the tool derives
everything else — including SPIR-V reflection metadata that becomes the source of
truth for the engine's vertex layout and resource bindings.

## Pipeline (per shader)

```
sprite2d.vs / sprite2d.fs            (canonical GLSL, #version 450)
        |  glslangValidator -V --target-env vulkan1.0
        v
   sprite2d.vertex.spv / sprite2d.fragment.spv      (SPIR-V: Vulkan, D3D12)
        |
        |-- spirv-cross --reflect   --> JSON --> metadata (inputs/ubos/samplers)
        |-- spirv-cross --hlsl      --> HLSL      (only for --backend d3d11)
        `-- spirv-cross --oglsl --es--> GLSL ES   (only for --backend opengles)
```

The binaries are embedded as **base85** strings, and the reflection metadata is
serialized next to them into `shaders_data.lua`.

## Requirements

On `PATH` **or** under `$VULKAN_SDK/Bin` (Windows: `%VULKAN_SDK%\Bin`):

- `glslangValidator` — GLSL → SPIR-V
- `spirv-cross` — SPIR-V → HLSL / GLSL ES, and `--reflect` JSON

Both ship with the Vulkan SDK. Python 3.8+ is required; **no pip packages** —
only the standard library (`subprocess`, `json`, `base64`, `argparse`, `re`, `os`).

## Usage

```sh
# list discovered shaders (pairs of .vs / .fs)
python build_shaders.py --list

# build ALL backend variants (default) into the plugin's shaders/ folder.
# Produces one self-contained <name>_data.lua per shader.
python build_shaders.py

# build only some shaders, add preprocessor defines
python build_shaders.py --shaders sprite2d --defines DEBUG,USE_SHADOWS=1

# build only ONE backend variant (e.g. just Vulkan SPIR-V)
python build_shaders.py --backend vulkan --out ./out

# override tool locations if not on PATH / not under VULKAN_SDK
python build_shaders.py --glsllang /path/to/glslangValidator --spirv_cross /path/to/spirv-cross
```

`--backend all` (default) emits every variant: `vulkan` (SPIR-V, base85),
`d3d11` (HLSL text), `opengl` (canonical GLSL text), `opengles` (GLSL ES text).

Outputs (default `--out` is `../../plugins/RenderBatch2D/shaders`):

- `plugins/RenderBatch2D/shaders/<name>_data.lua` — one file per shader.
- `plugins/RenderBatch2D/shaders/_work/` — intermediates (`.spv`, `.hlsl`,
  `.gles.glsl`) for debugging. Safe to delete; not loaded at runtime.

## Output format

One self-contained file per shader, with every backend variant + reflection.
Stages are keyed by the names `create_shader` accepts (`vertex`, `pixel`):

```lua
-- sprite2d_data.lua
return {
    vertex = {
        bin = {
            vulkan   = { enc = "base85", data = "<SPIR-V as base85>" },
            d3d11    = { enc = "text",   data = "<HLSL source>" },
            opengles = { enc = "text",   data = "<GLSL ES source>" },
            opengl   = { enc = "text",   data = "<desktop GLSL source>" },
        },
        meta = {
            inputs  = { { location = 0, name = "a_position", type = "float", glsl_type = "vec3", size = 3 }, ... },
            outputs = { { location = 0, name = "v_color", type = "float", glsl_type = "vec4", size = 4 }, ... },
            ubos    = { { set = 0, binding = 0, name = "Matrices", block_size = 64,
                          members = { { name = "world", type = "float", glsl_type = "mat4", offset = 0, size = 4 } } } },
            ssbos = {}, samplers = { { set = 0, binding = 1, name = "u_texture", glsl_type = "sampler2D" } }, images = {},
        },
    },
    pixel = { bin = { ... }, meta = { ... } },
}
```

Notes:

- `enc = "base85"` means `data` is RFC 1924 base85 (Python `base64.b85encode`);
  the Lua loader (`plugins/RenderBatch2D/shaders/init.lua`) decodes it to raw
  SPIR-V bytes. `enc = "text"` means the source is used as-is.
- `type` is the engine's scalar type name (matches `type_name_options`:
  `byte, ubyte, short, ushort, int, uint, float, double`); `size` is the
  component count (e.g. `vec3` → `type="float", size=3`).
- `glsl_type` keeps the original GLSL type string for reference.

## Consuming the data (RenderBatch2D)

The plugin does **not** branch on backend. `plugins/RenderBatch2D/shaders/init.lua`
loads the right variant based on `renderer:get_info().backend` (and `.es` for
OpenGL ES), decodes base85 SPIR-V when needed, and calls `renderer:create_shader`:

```lua
local shaders = require('plugins.RenderBatch2D.shaders')
local vs = shaders.get(renderer, 'sprite2d', 'vertex')  -- stage: 'vertex' | 'pixel'
```

`shaders.meta(name, stage)` returns the reflection metadata for future
pipeline-layout generation (Fase 3).

## Adding a shader

1. Drop `myname.vs` and `myname.fs` here (`#version 450`, explicit locations/bindings).
2. `python build_shaders.py` (regenerates every `<name>_data.lua`).
3. In the plugin, `shaders.get(renderer, 'myname', 'vertex'|'pixel')`.

## Known caveat for D3D11 (informs Phase 3)

`spirv-cross --hlsl` assigns its own input semantics (`TEXCOORD0/1/2`) instead of
the source variable names. The engine's D3D11 input layout currently uses the
attribute name as `SemanticName`, so Phase 3 will use the reflected metadata
(`inputs[].location`) to build a deterministic `location → semantic` mapping
(e.g. `TEXCOORD{location}`) so the HLSL and the input layout always agree. The
HLSL entry point is `main`; combined samplers map to `t{n}`/`s{n}` registers via
`binding`.
