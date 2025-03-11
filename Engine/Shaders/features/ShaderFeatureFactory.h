#pragma once
#include <memory>
#include <string>
#include "BaseShaderFeature.h"
#include "BlinnPhongFeature.h"
#include "Engine/TextureManager.h"

namespace FrostFireEngine
{
  class ShaderFeatureFactory {
  public:
    static std::unique_ptr<BaseShaderFeature> CreateFeature(
      const std::string& featureId,
      ID3D11Device*      device)
    {
      if (featureId == "BLINN_PHONG") {
        return std::make_unique<BlinnPhongFeature>(device);
      }
      // Ajouter les autres features ici
      return nullptr;
    }
  };
} // namespace FrostFireEngine
