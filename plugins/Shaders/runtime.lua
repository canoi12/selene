-- Shared shader runtime for plugins.Shaders.
--
-- Decodes embedded variants and creates an engine Shader for the renderer's
-- active backend. Per-shader init.lua modules build a data table and call
-- runtime.get; this module does not require any shader folder.

local runtime = {}

-- ---------------------------------------------------------------------------
-- Base85 (RFC 1924) decoder. Matches Python's base64.b85encode alphabet so the
-- SPIR-V embedded in bin.lua round-trips byte-for-byte.
-- ---------------------------------------------------------------------------

local B85_ALPHABET =
    '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ' ..
    'abcdefghijklmnopqrstuvwxyz!#$%&()*+-;<=>?@^_`{|}~'

local B85_DECODE = {}
for i = 1, #B85_ALPHABET do
    B85_DECODE[B85_ALPHABET:sub(i, i)] = i - 1
end

-- Decode an RFC 1924 base85 string (no padding) into a byte string.
function runtime.b85decode(str)
    local out = {}
    local acc, n = 0, 0
    for i = 1, #str do
        local d = B85_DECODE[str:sub(i, i)]
        if not d then
            error(('shaders.b85decode: invalid base85 char %q at %d')
                  :format(str:sub(i, i), i), 2)
        end
        acc = acc * 85 + d
        n = n + 1
        if n == 5 then
            for shift = 24, 0, -8 do
                out[#out + 1] = string.char((acc >> shift) & 0xFF)
            end
            acc, n = 0, 0
        end
    end
    -- Tail: 1..4 leftover chars encode 0..3 bytes.
    if n > 1 then
        for _ = 1, 5 - n do
            acc = acc * 85 + 84
        end
        local bytes = n - 1
        for shift = (bytes - 1) * 8, 0, -8 do
            out[#out + 1] = string.char((acc >> shift) & 0xFF)
        end
    elseif n == 1 then
        error('shaders.b85decode: truncated base85 group', 2)
    end
    return table.concat(out)
end

-- Map the renderer's backend (+ es flag) to a bin.lua key.
local function backend_key(info)
    local backend = info and info.backend
    if backend == 'opengl' then
        return info.es and 'opengles' or 'opengl'
    end
    -- 'vulkan' -> vulkan (SPIR-V); 'd3d11' -> d3d11 (HLSL). Pass through.
    return backend
end

local function shader_name(data)
    return data and data.name or '?'
end

-- Return the raw (decoded) blob string for a given data table/stage/backend key.
local function variant_blob(data, stage, key)
    local st = data[stage]
    if not st or not st.bin then
        error(('shaders: shader %q has no stage %q'):format(shader_name(data), stage), 3)
    end
    local v = st.bin[key]
    if not v then
        local have = {}
        for k in pairs(st.bin) do have[#have + 1] = k end
        table.sort(have)
        error(('shaders: shader %q stage %q has no variant for backend %q (have: %s)')
              :format(shader_name(data), stage, key, table.concat(have, ', ')), 3)
    end
    if v.enc == 'base85' then
        return runtime.b85decode(v.data)
    end
    return v.data  -- text variants (glsl / hlsl)
end

-- Create an engine Shader for the renderer's active backend.
--   data: { name = string, vertex = { bin, meta }, pixel = { bin, meta } }
--   stage: 'vertex' | 'pixel'
function runtime.get(renderer, data, stage)
    if type(renderer) ~= 'userdata' or type(renderer.get_info) ~= 'function' then
        error('shaders.get: expected a renderer', 2)
    end
    if stage ~= 'vertex' and stage ~= 'pixel' then
        error("shaders.get: stage must be 'vertex' or 'pixel'", 2)
    end
    local info = renderer:get_info()
    local key = backend_key(info)
    local blob = variant_blob(data, stage, key)
    return renderer:create_shader(stage, blob)
end

-- Return the reflection metadata table for a stage of an already-loaded data table.
function runtime.meta(data, stage)
    if stage ~= 'vertex' and stage ~= 'pixel' then
        error("shaders.meta: stage must be 'vertex' or 'pixel'", 2)
    end
    local st = data[stage]
    if not st then
        error(('shaders: shader %q has no stage %q'):format(shader_name(data), stage), 2)
    end
    return st.meta
end

return runtime
