#include "ShaderTechniqueFactory.h"
#include "GlobaleTechnique.h"
#include "TextTechnique.h"
#include "Engine/ECS/components/rendering/PBRRenderer.h"

namespace FrostFireEngine
{
  std::unique_ptr<ShaderTechnique> ShaderTechniqueFactory::CreateTechnique(
    const std::string& techniqueName)
  {
    if (techniqueName == "Globale") {
      return std::make_unique<GlobaleTechnique>();
    }
    else if (techniqueName == "Text") {
      return std::make_unique<TextTechnique>();
    }

    return nullptr;
  }
}
