#include "LightingCommon.hlsli"

cbuffer MatrixBuffer : register(b0)
{
    float4x4 modelViewProjection;
    float4x4 world;
    float4x4 worldInverseTranspose;
};

cbuffer MaterialBuffer : register(b1)
{
    float4 diffuseColor;    // rgb + alpha
    float4 specularColor;   // rgb + shininess
    float4 ambientColorMat; // rgb + unused
};

Texture2D gDiffuse : register(t0);
SamplerState gSampler : register(s0);

struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 positionH : SV_POSITION;
    float2 uv        : TEXCOORD;
    float3 normalW   : NORMAL;
    float3 worldPos  : TEXCOORD1;
};

VS_OUTPUT VS_Transparency(VS_INPUT input)
{
    VS_OUTPUT output;

    output.positionH = mul(float4(input.position, 1.0f), modelViewProjection);

    float4 posW = mul(float4(input.position, 1.0f), world);
    output.worldPos = posW.xyz;

    output.uv = input.uv;
    float3 normalW = mul(input.normal, (float3x3)worldInverseTranspose);
    output.normalW = normalize(normalW);

    return output;
}

struct PS_OUTPUT
{
    float4 color : SV_Target;
};

[earlydepthstencil]
PS_OUTPUT PS_Transparency(VS_OUTPUT input)
{
    PS_OUTPUT output;

    float4 baseColor = gDiffuse.Sample(gSampler, input.uv);

    float3 N = normalize(input.normalW);

  float3 materialDiffuse = diffuseColor.rgb * baseColor.rgb;
  float shininess = specularColor.w;

  float3 V = normalize(cameraPosition - input.worldPos);
  float3 L;
  float3 H;

  if (baseColor.a < 0.1)
  {
    discard;
  }
  float3 finalColor = float3(0.0f, 0.0f, 0.0f);
  for (int i = 0; i < 64; i++)
  {
    // On suppose que l'on utilise la première lumière du tableau comme éclairage principal.
    float3 mainLightDirection = lights[i].directionType.xyz;
    float mainLightIntensity = lights[i].colorIntensity.w;
    float3 mainLightColor = lights[i].colorIntensity.xyz;


    if (lights[i].directionType.w == 0.0f && mainLightIntensity > 0.0f)
    {
    
      L = normalize(-mainLightDirection);
      H = normalize(L + V);
     
      

     
    }
    else if (mainLightIntensity > 0.0f)
    {
      float3 delta = lights[i].positionRange.xyz - input.worldPos;
      mainLightIntensity = 2.0;
      if (length(delta) <= lights[i].positionRange.w)
      {
        L = normalize(delta);
        H = normalize(L + V);
        N = abs(N);
        float attenuation = 1.0 / (1.0 + 0.1 * length(delta) + 0.01 * (length(delta) * length(delta)));
        mainLightIntensity *= attenuation;
      }
      else
      {
        continue;
      }

    }
    else
    {
      continue;
    }

    float3 finalColorInt = PBRLighting(
        input.worldPos,
        N,
        materialDiffuse,
        cameraPosition,
        mainLightDirection,
        mainLightIntensity,
        mainLightColor
    );

    finalColor += finalColorInt + ambientColorMat.rgb * materialDiffuse * 0.1f;

  }
    

    output.color = float4(finalColor, diffuseColor.a);
    return output;
}
