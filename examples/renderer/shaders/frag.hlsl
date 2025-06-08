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