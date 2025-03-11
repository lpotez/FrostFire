cbuffer MatrixBuffer : register(b0) {
    float4x4 viewProj;
}

struct VS_INPUT {
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD;
};

struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float3 texCoord : TEXCOORD0;
};

VS_OUTPUT VS_Skybox(VS_INPUT input) {
    VS_OUTPUT output;

    float4 pos = mul(float4(input.position, 1.0f), viewProj);

    // Place le vertex sur le plan lointain
    output.position = float4(pos.xyw, pos.w);

    // Normaliser le vecteur directionnel pour le cube map
    output.texCoord = normalize(input.position);

    return output;
}

TextureCube skyboxTexture : register(t0);
SamplerState skyboxSampler : register(s0);

float4 PS_Skybox(VS_OUTPUT input) : SV_Target {
    float3 direction = normalize(input.texCoord);
    return skyboxTexture.Sample(skyboxSampler, direction);
}
