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
    output.position = float4(input.position, 1.0);
    output.color = input.color;
    output.texcoord = input.texcoord;
    return output;
}