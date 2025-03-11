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

Texture2D fontAtlas : register(t0);
SamplerState fontSampler : register(s0)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

struct VS_INPUT
{
    float2 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VS_OUTPUT VS_Text(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = mul(float4(input.position, 0.0f, 1.0f), modelViewProjection);
    output.uv = input.uv;
    return output;
}

float4 PS_Text(VS_OUTPUT input) : SV_TARGET
{
    float4 sampled = fontAtlas.Sample(fontSampler, input.uv);
    return float4(uiColor.rgb, sampled.r);
}

BlendState TextBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    SrcBlend[0] = SRC_ALPHA;
    DestBlend[0] = INV_SRC_ALPHA;
    BlendOp[0] = ADD;
    SrcBlendAlpha[0] = ONE;
    DestBlendAlpha[0] = INV_SRC_ALPHA;
    BlendOpAlpha[0] = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
    StencilEnable = FALSE;
};

RasterizerState TextRasterizer
{
    CullMode = None;
    FillMode = Solid;
};

technique11 TextTechnique
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS_Text()));
        SetPixelShader(CompileShader(ps_5_0, PS_Text()));
        SetBlendState(TextBlending, float4(0,0,0,0), 0xffffffff);
        SetDepthStencilState(DisableDepth, 0);
        SetRasterizerState(TextRasterizer);
    }
}
