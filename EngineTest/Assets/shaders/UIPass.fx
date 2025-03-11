cbuffer TransformBuffer : register(b0)
{
    float4x4 modelViewProjection;
    float4x4 world;
    float4x4 worldInverseTranspose;
};

cbuffer ColorBuffer : register(b1)
{
    float4 uiColor;
};

struct VSInput {
    float2 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VSOutput VS_UI(VSInput input)
{
    VSOutput output;
    output.position = mul(float4(input.pos,0,1), modelViewProjection);
    output.uv = input.uv;
    return output;
}

Texture2D diffuseTex : register(t0);
SamplerState samLinear : register(s0);

float4 PS_UI(VSOutput input) : SV_TARGET
{
    float4 texColor = diffuseTex.Sample(samLinear, input.uv);
    return texColor * uiColor;
}
