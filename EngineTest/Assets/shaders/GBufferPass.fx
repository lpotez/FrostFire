cbuffer MatrixBuffer : register(b0)
{
    float4x4 modelViewProjection;
    float4x4 world;
    float4x4 worldInverseTranspose;
}

cbuffer MaterialBuffer : register(b1)
{
    float4 baseColor;
    float metallic;
    float roughness;
    float ao;
    float padding;
}

Texture2D gAlbedoTex            : register(t0);
Texture2D gNormalTex            : register(t1);
Texture2D gMetallicRoughnessTex : register(t2);
Texture2D gAOTex                : register(t3);
SamplerState gSampler : register(s0);

struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
    float3 tangent  : TANGENT;
};

struct VS_OUTPUT
{
    float4 positionH : SV_POSITION;
    float2 uv        : TEXCOORD0;
    float3 normalW   : NORMAL;
    float3 worldPos  : TEXCOORD1;
    float3 tangentW  : TEXCOORD2;
    float3 bitangentW: TEXCOORD3;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 posW = mul(float4(input.position, 1.0f), world);
    output.positionH = mul(float4(input.position, 1.0f), modelViewProjection);
    output.worldPos = posW.xyz;
    float3 N = normalize(mul(input.normal, (float3x3)worldInverseTranspose));
    float3 T = normalize(mul(input.tangent, (float3x3)worldInverseTranspose));
    float3 B = normalize(cross(N, T));
    output.normalW = N;
    output.tangentW = T;
    output.bitangentW = B;
    output.uv = input.uv;
    return output;
}

struct PS_OUTPUT
{
    float4 position : SV_TARGET0;
    float4 normal   : SV_TARGET1;
    float4 albedo   : SV_TARGET2;
};

[earlydepthstencil]
PS_OUTPUT PS_GBuffer(VS_OUTPUT input)
{
    PS_OUTPUT output;
    float4 albedoSample = gAlbedoTex.Sample(gSampler, input.uv) * baseColor;
    float4 normalMapSample = gNormalTex.Sample(gSampler, input.uv);
    float3 normalMap = normalMapSample.xyz * 2.0f - 1.0f;
    float3 N = normalize(float3(
        dot(normalMap, float3(input.tangentW.x, input.bitangentW.x, input.normalW.x)),
        dot(normalMap, float3(input.tangentW.y, input.bitangentW.y, input.normalW.y)),
        dot(normalMap, float3(input.tangentW.z, input.bitangentW.z, input.normalW.z))
    ));
    float4 mrSample = gMetallicRoughnessTex.Sample(gSampler, input.uv);
    float M = metallic * mrSample.r;
    float R = roughness * mrSample.g;
    float aoSample = gAOTex.Sample(gSampler, input.uv).r;
    float A = ao * aoSample;
    output.position = float4(input.worldPos, M);
    output.normal = float4(normalize(N), R);
    output.albedo = float4(albedoSample.rgb, A);
    return output;
}
