#pragma once
#include <string>
#include <map>
#include <vector>
#include "Engine/Shaders/ShaderVariant.h"
#include "Engine/Shaders/features/RenderPass.h"

namespace FrostFireEngine
{
  // Informations nécessaires pour compiler un shader pour une passe donnée
  struct PassShaderInfo {
    std::wstring                       shaderFile; // Chemin vers le HLSL
    std::string                        vsEntryPoint;
    std::string                        psEntryPoint;
    std::map<std::string, std::string> defines; // Defines spécifiques à cette passe
  };

  // Interface de base d'une technique
  class ShaderTechnique {
  public:
    virtual ~ShaderTechnique() = default;

    // Chaque technique doit fournir, pour une passe donnée, les infos pour compiler/obtenir un ShaderVariant
    virtual bool GetPassShaderInfo(RenderPass pass, PassShaderInfo& outInfo) const = 0;

    // Nom de la technique, utilisé pour le caching dans le shader manager si besoin
    virtual std::string GetTechniqueName() const = 0;

    // Récupère un variant pour cette technique, la passe donnée, et les features
    // Cette fonction utilise le ShaderManager pour obtenir un ShaderVariant compilé
    virtual ShaderVariant* GetVariantForPass(RenderPass pass,
                                             const std::vector<std::string>&
                                             features,
                                             const VertexLayoutDesc& layout) const;
  };
}
