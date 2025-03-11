#pragma once
#include <d3d11.h>
#include "Engine/DispositifD3D11.h"

namespace FrostFireEngine
{
  // Crée et retourne une SRV pour une texture 1x1 blanche 2D
  ID3D11ShaderResourceView* CreateWhiteTexture(ID3D11Device* device);

  // Initialise la texture blanche fallback 2D dans le TextureManager, si pas déjà présente
  void InitializeWhiteFallbackTexture(DispositifD3D11* device);

  // Crée et retourne une SRV pour une cube map 1x1 blanche (6 faces blanches)
  ID3D11ShaderResourceView* CreateWhiteCubeMap(ID3D11Device* device);

  // Initialise la cube map blanche fallback dans le TextureManager, si pas déjà présente
  void InitializeWhiteCubeMapFallbackTexture(DispositifD3D11* device);

  // Crée et retourne une SRV pour une texture 1x1 représentant une normale neutre (0.5,0.5,1.0)
  ID3D11ShaderResourceView* CreateNeutralNormalTexture(ID3D11Device* device);

  // Initialise la texture neutre de fallback pour les normales, si pas déjà présente
  void InitializeNeutralNormalFallbackTexture(DispositifD3D11* device);
}
