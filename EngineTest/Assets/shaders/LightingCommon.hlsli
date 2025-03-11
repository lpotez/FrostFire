#define MAX_LIGHTS 64

struct GPU_Light {
  float4 colorIntensity; // xyz = color, w = intensity
  float4 directionType; // xyz = direction, w = type (0 = Dir, 1 = Point, 2 = Spot)
  float4 positionRange; // xyz = position, w = range
  float4 spotAnglePad;
};

cbuffer LightBuffer : register(b3) {
  float3    cameraPosition;
  float     lightCount;
  GPU_Light lights[MAX_LIGHTS];
}

float3 BlinnPhongLighting(float3 positionW, float3 normalW, float3 albedo, float3 cameraPos, float3 lightDir, float lightIntensity, float3 lightColor)
{
  float3 L = normalize(-lightDir);
  float3 V = normalize(cameraPos - positionW);
  float3 H = normalize(L + V);
  float  NdotL = saturate(dot(normalW, L));
  float  NdotH = saturate(dot(normalW, H));
  float  shininess = 32.0f;
  float  specularTerm = pow(NdotH, shininess);
  float3 diffuse = albedo * NdotL * lightIntensity * lightColor;
  float3 specular = float3(1, 1, 1) * specularTerm * lightIntensity * lightColor;
  float3 ambient = 0.05f * albedo;
  return ambient + diffuse + specular;
}

float3 PBRLighting(float3 positionW, float3 normalW, float3 albedo, float3 cameraPos, float3 lightDir, float lightIntensity, float3 lightColor)
{
  float3 L = normalize(-lightDir);
  float3 V = normalize(cameraPos - positionW);
  float  NdotL = saturate(dot(normalW, L));
  float3 diffuse = albedo * NdotL * lightIntensity * lightColor;
  float3 ambient = 0.05f * albedo;
  return ambient + diffuse;
}
