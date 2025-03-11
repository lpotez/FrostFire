#pragma once
#include "ShaderTechnique.h"
#include <unordered_map>

namespace FrostFireEngine
{
  class GlobaleTechnique : public ShaderTechnique {
  public:
    GlobaleTechnique();
    bool        GetPassShaderInfo(RenderPass pass, PassShaderInfo& outInfo) const override;
    std::string GetTechniqueName() const override
    {
      return "Globale";
    }

  private:
    std::unordered_map<RenderPass, PassShaderInfo> m_passInfos;
  };
}
