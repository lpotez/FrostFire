#include "ShaderTechnique.h"
#include "Engine/Shaders/ShaderManager.h"

namespace FrostFireEngine
{
  ShaderVariant* ShaderTechnique::GetVariantForPass(RenderPass                      pass,
                                                    const std::vector<std::string>& features,
                                                    const VertexLayoutDesc&         layout) const
  {
    PassShaderInfo info;
    if (!GetPassShaderInfo(pass, info)) {
      return nullptr;
    }

    // On combine les defines de la technique pour cette passe avec les features
    std::map<std::string, std::string> finalDefines = info.defines;
    // Les features peuvent être traitées comme des defines boolean
    for (const auto& f : features) {
      finalDefines[f] = "1";
    }

    // On utilise le ShaderManager (singleton) pour obtenir un variant
    ShaderManager& manager = ShaderManager::GetInstance();
    ShaderVariant* variant = manager.GetOrCreatePassVariant(
      GetTechniqueName(),
      pass,
      info.shaderFile,
      info.vsEntryPoint,
      info.psEntryPoint,
      finalDefines,
      layout
    );

    return variant;
  }
}
