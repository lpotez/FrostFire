  cbuffer LightMatrixBuffer : register(b1)
  {
      float4x4 lightViewProjection;
  }

  cbuffer MatrixBuffer : register(b0)
  {
      float4x4 modelViewProjection;
      float4x4 world;
      float4x4 worldInverseTranspose;
  }


  struct VS_INPUT
  {
      float3 position : POSITION;
      float3 normal   : NORMAL;
      float2 uv       : TEXCOORD0;
  };

  struct VS_OUTPUT
  {
      float4 positionH : SV_POSITION;
  };

  VS_OUTPUT VS_Shadow(VS_INPUT input)
  {
      VS_OUTPUT output;

      // Transforme la position du vertex de l’espace local du modèle vers l’espace monde
      float4 worldPos = mul(float4(input.position, 1.0f), world);

      // Puis projette la position monde dans l’espace de la lumière
      output.positionH = mul(worldPos, lightViewProjection);

      return output;
  }

  // Le pixel shader n’écrit pas de couleur, juste la profondeur via la SV_Depth.
  float4 PS_Shadow() : SV_TARGET
  {
      return float4(1.0f,1.0f,1.0f,1.0f);
  }
