#pragma once
#include <string>
#include <memory>

#include "ShaderTechnique.h"

namespace FrostFireEngine
{
  class ShaderTechniqueFactory {
  public:
    static std::unique_ptr<ShaderTechnique> CreateTechnique(const std::string& techniqueName);
  };
}
