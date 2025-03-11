Texture2D gLightingTex : register(t0);
SamplerState gSampler : register(s0);

struct VS_OUT {
    float4 positionH : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VS_OUT VS(uint vertId : SV_VertexID)
{
    VS_OUT output;
    float2 corners[3] = {
      float2(-1.0f, -1.0f),
      float2(-1.0f,  3.0f),
      float2( 3.0f, -1.0f),
    };

    float2 pos = corners[vertId];
    output.positionH = float4(pos, 0, 1);

    output.uv = (pos * 0.5f) + float2(0.5f,0.5f);
    output.uv.y = 1.0f - output.uv.y;

    return output;
}

float4 PS_Final(VS_OUT input) : SV_Target
{
    float2 uv = input.uv;
    float4 lightingColor = gLightingTex.Sample(gSampler, uv);
    return lightingColor;
}
