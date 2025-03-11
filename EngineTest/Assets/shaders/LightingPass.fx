#define MAX_LIGHTS 64
#define PI 3.14159265359
#define SHADOW_MAP_SIZE 4096.0f
#define LIGHT_SIZE 0.005f

struct GPU_Light {
    float4 colorIntensity;
    float4 directionType;
    float4 positionRange;
    float4 spotAnglePad;
};

cbuffer LightBuffer : register(b3)
{
    float3    cameraPosition;
    float     lightCount;
    GPU_Light lights[MAX_LIGHTS];
}

cbuffer ScreenSizeBuffer : register(b1)
{
    float screenWidth;
    float screenHeight;
    float padX;
    float padY;
}

cbuffer LightMatrixBufferArray : register(b4)
{
    float4x4 gLightViewProjectionArray[4];
}

cbuffer FrameData : register(b5)
{
    float shadowPoissonRotation;
    float padFD1;
    float padFD2;
    float padFD3;
}

Texture2D gPositionTex : register(t0);
Texture2D gNormalTex   : register(t1);
Texture2D gAlbedoTex   : register(t2);

SamplerState gSamplerLinear : register(s0);

Texture2DArray<float> gShadowMapArray : register(t3);
SamplerComparisonState gShadowSampler : register(s1);

TextureCube gIrradianceMap     : register(t4);
TextureCube gPrefilteredEnvMap : register(t5);
Texture2D   gBRDFLUT           : register(t6);

static const float2 poissonDisk[24] = {
    float2(-0.706, -0.706), float2(-0.5,   -0.866), float2(-0.259, -0.966), float2(0.0,   -1.0),
    float2(0.259, -0.966),  float2(0.5,    -0.866), float2(0.706,  -0.706), float2(0.866, -0.5),
    float2(0.966, -0.259),  float2(1.0,     0.0),   float2(0.966,  0.259),  float2(0.866, 0.5),
    float2(0.706, 0.706),   float2(0.5,     0.866), float2(0.259,  0.966),  float2(0.0,   1.0),
    float2(-0.259, 0.966),  float2(-0.5,    0.866), float2(-0.706, 0.706),  float2(-0.866,0.5),
    float2(-0.966,0.259),   float2(-1.0,    0.0),   float2(-0.966,-0.259),  float2(-0.866,-0.5)
};

static const float gaussianWeights[24] = {
    0.018, 0.02, 0.022, 0.024, 0.026, 0.028,
    0.03, 0.03, 0.03, 0.03, 0.03, 0.03,
    0.03, 0.03, 0.03, 0.03, 0.03, 0.03,
    0.028,0.026,0.024,0.022,0.02, 0.018
};

float2 RotatePoissonSample(float2 offset, float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return float2(offset.x*c - offset.y*s, offset.x*s + offset.y*c);
}

float2 CalcShadowTexCoord(float4 posWorldSpace, int lightIndex)
{
    float4 posLightSpace = mul(posWorldSpace, gLightViewProjectionArray[lightIndex]);
    float2 projCoords = posLightSpace.xy / posLightSpace.w;
    float2 shadowTexCoord;
    shadowTexCoord.x = 0.5f * projCoords.x + 0.5f;
    shadowTexCoord.y = -0.5f * projCoords.y + 0.5f;
    return shadowTexCoord;
}

float AverageBlockerDepth(float2 shadowTexCoord, float currentDepth, float searchRadius, int lightIndex, out int blockersFound)
{
    blockersFound = 0;
    float blockerSum = 0.0f;
    [unroll]
    for (int i = 0; i < 24; i++)
    {
        float2 rotatedOffset = RotatePoissonSample(poissonDisk[i], shadowPoissonRotation);
        float2 offset = rotatedOffset * searchRadius;
        float smDepth = gShadowMapArray.SampleLevel(gSamplerLinear, float3(shadowTexCoord + offset, lightIndex), 0);
        if (smDepth < currentDepth)
        {
            blockerSum += smDepth * gaussianWeights[i];
            blockersFound++;
        }
    }
    if (blockersFound > 0)
        return blockerSum;
    return 1.0f;
}

float PCSSShadow(float4 posWorldSpace, int lightIndex)
{
    float4 posLightSpace = mul(posWorldSpace, gLightViewProjectionArray[lightIndex]);
    float currentDepth = posLightSpace.z / posLightSpace.w;
    float bias = 0.0005f;
    currentDepth -= bias;

    float2 shadowTexCoord = CalcShadowTexCoord(posWorldSpace, lightIndex);
    if (shadowTexCoord.x < 0.0f || shadowTexCoord.x > 1.0f || shadowTexCoord.y < 0.0f || shadowTexCoord.y > 1.0f)
        return 1.0f;

    float distanceFromLight = posLightSpace.w;
    float searchRadius = LIGHT_SIZE * (distanceFromLight / currentDepth) * (1.0f / SHADOW_MAP_SIZE);

    int blockersFound;
    float avgBlockerDepth = AverageBlockerDepth(shadowTexCoord, currentDepth, searchRadius, lightIndex, blockersFound);

    if (blockersFound == 0)
        return 1.0f;

    float penumbraRatio = (currentDepth - avgBlockerDepth) / avgBlockerDepth;
    float filterRadius = penumbraRatio * LIGHT_SIZE * (distanceFromLight / currentDepth) * (1.0f / SHADOW_MAP_SIZE);

    float shadow = 0.0f;
    float weightSum = 0.0f;
    [unroll]
    for (int i = 0; i < 24; i++)
    {
        float2 rotatedOffset = RotatePoissonSample(poissonDisk[i], shadowPoissonRotation);
        float2 offset = rotatedOffset * filterRadius;
        float sampleVal = gShadowMapArray.SampleCmpLevelZero(gShadowSampler, float3(shadowTexCoord + offset, lightIndex), currentDepth);
        shadow += sampleVal * gaussianWeights[i];
        weightSum += gaussianWeights[i];
    }

    shadow /= weightSum;
    return 0.7 * shadow;
}

float DistributionGGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a*a;
    float denom = (NdotH*NdotH)*(a2-1.0f)+1.0f;
    return a2/(PI*denom*denom);
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float r = (roughness+1.0f);
    float k = (r*r)/8.0f;
    float ggx1 = NdotV/(NdotV*(1.0f-k)+k);
    float ggx2 = NdotL/(NdotL*(1.0f-k)+k);
    return ggx1*ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0)*pow(1.0f - cosTheta, 5.0f);
}

float3 GetIBLContribution(float3 N, float3 V, float roughness, float3 F0, float3 albedo, float metallic, float ao)
{
    float3 irradiance = gIrradianceMap.Sample(gSamplerLinear, N).rgb;
    float3 diffuse = irradiance * albedo * (1.0f - F0) * (1.0f - metallic);

    float NdotV = saturate(dot(N,V));
    float3 R = reflect(-V, N);
    float maxLOD = 5.0f;
    float lod = roughness * maxLOD;
    float3 prefilteredColor = gPrefilteredEnvMap.SampleLevel(gSamplerLinear, R, lod).rgb;

    float2 brdf = gBRDFLUT.Sample(gSamplerLinear, float2(NdotV, roughness)).rg;
    float3 F = FresnelSchlick(NdotV, F0);
    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    float3 ambient = (diffuse + specular)*ao;
    return ambient;
}

struct VS_OUT
{
    float4 positionH : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VS_OUT VS(uint vertId : SV_VertexID)
{
    VS_OUT output;
    float2 corners[3] = {
        float2(-1.0f,-1.0f),
        float2(-1.0f, 3.0f),
        float2( 3.0f,-1.0f)
    };
    float2 pos = corners[vertId];
    output.positionH = float4(pos, 0.0f, 1.0f);
    output.uv = (pos * 0.5f) + float2(0.5f,0.5f);
    output.uv.y = 1.0f - output.uv.y;
    return output;
}

[earlydepthstencil]
float4 PS_Lighting(VS_OUT input) : SV_Target
{
    float2 uv = input.uv;

    float4 posSample    = gPositionTex.Sample(gSamplerLinear, uv);
    float4 normalSample = gNormalTex.Sample(gSamplerLinear, uv);
    float4 albedoSample = gAlbedoTex.Sample(gSamplerLinear, uv);

    float3 positionW = posSample.xyz;
    float3 normalW   = normalize(normalSample.xyz);
    float  metallic   = saturate(posSample.w);
    float  roughness  = saturate(normalSample.w);
    float3 albedo     = albedoSample.rgb;
    float  ao         = saturate(albedoSample.w);

    float3 N = normalW;
    float3 V = normalize(cameraPosition - positionW);
    float NdotV = saturate(dot(N,V));

    float3 F0 = float3(0.04f,0.04f,0.04f);
    F0 = lerp(F0, albedo, metallic);

    float3 Lo = 0.0f;
    int count = (int)lightCount;

    [loop]
    for (int i = 0; i < count; i++)
    {
        float3 color     = lights[i].colorIntensity.rgb;
        float  intensity = lights[i].colorIntensity.w;

        float3 direction = lights[i].directionType.xyz;
        float  type      = lights[i].directionType.w;

        float3 lightPos  = lights[i].positionRange.xyz;
        float  range     = lights[i].positionRange.w;

        float  spotAngle = lights[i].spotAnglePad.x;

        float3 L;
        if (type == 0.0f) {
            L = normalize(-direction);
        } else if (type == 1.0f) {
            float3 toLight = lightPos - positionW;
            float dist = length(toLight);
            if (dist > range) intensity = 0;
            L = toLight/dist;
            intensity *= saturate(1.0f - dist/range);
        } else {
            float3 toLight = lightPos - positionW;
            float dist = length(toLight);
            if (dist > range) intensity = 0;
            L = toLight/dist;
            float spotCos = dot(L, -normalize(direction));
            float spotThreshold = cos(radians(spotAngle*0.5f));
            if (spotCos < spotThreshold) intensity = 0;
            else {
                float atten = saturate(1.0f - dist/range);
                float spotSmooth = (spotCos - spotThreshold)/(1.0f - spotThreshold);
                intensity *= atten*spotSmooth;
            }
        }

        if (intensity > 0.0f) {
            float3 H = normalize(V+L);
            float NdotL = saturate(dot(N,L));
            float NdotH = saturate(dot(N,H));
            float VdotH = saturate(dot(V,H));

            float D = DistributionGGX(NdotH, roughness);
            float G = GeometrySmith(NdotV, NdotL, roughness);
            float3 F = FresnelSchlick(VdotH, F0);

            float3 kS = F;
            float3 kD = (1.0f - kS)*(1.0f - metallic);

            float3 numerator = D*G*F;
            float denominator = 4.0f*NdotV*NdotL+0.0001f;
            float3 specular = numerator/denominator;

            float3 diffuse = kD*albedo/PI;
            float3 contrib = (diffuse+specular)*NdotL*intensity*color;

            float shadowFactor=1.0f;
            if (type == 0.0f && i<4) {
                shadowFactor = PCSSShadow(float4(positionW,1.0f), i);
            }

            Lo += contrib*shadowFactor;
        }
    }

    float3 ibl = GetIBLContribution(N, V, roughness, F0, albedo, metallic, ao);

    float3 finalColor = Lo + ibl;

    const float a=2.51f;
    const float b=0.03f;
    const float c=2.43f;
    const float d=0.59f;
    const float e=0.14f;
    finalColor = (finalColor*(a*finalColor+b))/(finalColor*(c*finalColor+d)+e);

    finalColor = pow(finalColor, 1.0f/2.2f);

    return float4(finalColor,1.0f);
}

technique11 PBRLightingTech
{
    pass P0
    {
        SetVertexShader( CompileShader(vs_5_0, VS()) );
        SetPixelShader( CompileShader(ps_5_0, PS_Lighting()) );
    }
}
