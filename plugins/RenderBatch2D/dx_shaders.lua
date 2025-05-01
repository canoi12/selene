local shaders = {}

local vs_sources = {}
local ps_sources = {}

-- Sprite2D shaders
vs_sources['SPRITE2D'] = [[
cbuffer Matrix : register(b0) {
    matrix world;
};

struct VSInput
{
    float3 position : a_position;
    float4 color : a_color;
    float2 texcoord : a_texcoord0;
};

struct PSInput {
    float4 position : SV_POSITION;
    float4 color : a_color;
    float2 texcoord : a_texcoord0;
};

PSInput main(VSInput input) {
    PSInput output;
    output.position = mul(world, float4(input.position, 1.0));
    output.color = input.color;
    output.texcoord = input.texcoord;
    return output;
}
]]

ps_sources['SPRITE2D'] =
[[
Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

struct PSInput {
    float4 position : SV_POSITION;
    float4 color : a_color;
    float2 texcoord : a_texcoord0;
};

float4 main(PSInput input) : SV_TARGET {
    float4 pixelColor = input.color * shaderTexture.Sample(SampleType, input.texcoord);
    return pixelColor;
}
]]

-- Primitive 2D shaders
vs_sources['PRIMITIVE2D'] = [[
cbuffer Matrix : register(b0) {
    matrix world;
};

struct VSInput
{
    float3 position : a_position;
    float4 color : a_color;
};

struct PSInput {
    float4 position : SV_POSITION;
    float4 color : a_color;
};

PSInput main(VSInput input) {
    PSInput output;
    float4 pos = float4(input.position, 1.0);
    output.position = mul(world, pos);
    output.color = input.color;
    return output;
}
]]

ps_sources['PRIMITIVE2D'] =
[[
struct PSInput {
    float4 position : SV_POSITION;
    float4 color : a_color;
};

float4 main(PSInput input) : SV_TARGET {
    return input.color;
}
]]

function shaders.compile_all(renderer)
    local list = {
        ['SPRITE2D'] = {
            vertex = renderer:create_shader('vertex', vs_sources['SPRITE2D']),
            pixel = renderer:create_shader('pixel', ps_sources['SPRITE2D'])
        },
        ['PRIMITIVE2D'] = {
            vertex = renderer:create_shader('vertex', vs_sources['PRIMITIVE2D']),
            pixel = renderer:create_shader('pixel', ps_sources['PRIMITIVE2D'])
        }
    }

    return list
end

return shaders