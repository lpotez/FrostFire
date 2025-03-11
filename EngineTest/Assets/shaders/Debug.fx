cbuffer MatrixBuffer : register(b0)
{
    float4x4 gWorldViewProj;
};

struct VSInput {
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VSOutput {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VSOutput VS_Debug(VSInput input) {
    VSOutput output;
    output.pos = mul(float4(input.pos, 1.0f), gWorldViewProj);
    output.color = input.color;
    return output;
}

float4 PS_Debug(VSOutput input) : SV_TARGET {
    return input.color;
}
