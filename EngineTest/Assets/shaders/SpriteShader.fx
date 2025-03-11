Texture2D spriteTexture;
SamplerState samplerState;

struct VertexIn
{
  float3 pos : POSITION;
  float2 tex : TEXCOORD;
};

struct PixelIn
{
  float4 pos : SV_POSITION;
  float2 tex : TEXCOORD;
};

PixelIn VS(VertexIn vin)
{
  PixelIn pout;
  pout.pos = float4(vin.pos, 1.0f); // Transformation de position
  pout.tex = vin.tex; // Coordonnées de texture
  return pout;
}

float4 PS(PixelIn pin) : SV_TARGET
{
  return spriteTexture.Sample(samplerState, pin.tex); // Échantillonnage de la texture
}

technique10 SpriteTech
{
  pass P0
  {
    SetVertexShader(CompileShader(vs_5_0, VS()));
    SetPixelShader(CompileShader(ps_5_0, PS()));
  }
}
