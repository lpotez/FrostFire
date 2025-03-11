Texture2D gInputTex : register(t0);
SamplerState gSampler : register(s0);

struct VS_OUT {
    float4 positionH : SV_POSITION; // Position en coordonnées écran
    float2 uv        : TEXCOORD0;   // Coordonnées UV
};

VS_OUT VS_PostProcess(uint vertId : SV_VertexID)
{
    VS_OUT output;

    // Triangle fullscreen couvrant tout l'écran
    float2 corners[3] = {
        float2(-1.0f, -1.0f),
        float2(-1.0f,  3.0f),
        float2( 3.0f, -1.0f),
    };

    float2 pos = corners[vertId];
    output.positionH = float4(pos, 0, 1);

    // Conversion [-1,1] en [0,1] pour UV
    output.uv = (pos * 0.5f) + float2(0.5f, 0.5f);
    // Inversion du Y si nécessaire, ici on inverse pour correspondre à la convention texture
    output.uv.y = 1.0f - output.uv.y;

    return output;
}

float4 PS_Vignette(VS_OUT input) : SV_Target
{
    float2 uv = input.uv;
    float4 color = gInputTex.Sample(gSampler, uv);

    // Paramètres du vignette
    float radius = 0.75;
    float softness = 0.45;

    // Distance du centre (0.5,0.5)
    float dist = distance(uv, float2(0.5, 0.5));

    float vignette = smoothstep(radius - softness, radius, dist);

    // On mélange la couleur avec du noir en fonction de "vignette"
    // Au centre dist < (radius - softness), vignette ~0, donc presque pas d'assombrissement.
    // Aux bords dist > radius, vignette ~1, donc assombrissement fort.
    color.rgb = lerp(color.rgb, 0.0f, vignette);

    return color;
}
