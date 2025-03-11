#pragma once
#include "Engine/Shaders/techniques/ShaderTechnique.h"
#include "Engine/Shaders/features/RenderPass.h"
#include <map>

namespace FrostFireEngine
{
  class TextTechnique : public ShaderTechnique {
  public:
    TextTechnique();
    bool        GetPassShaderInfo(RenderPass pass, PassShaderInfo& outInfo) const override;
    std::string GetTechniqueName() const override
    {
      return "Text";
    }

  private:
    std::map<RenderPass, PassShaderInfo> m_passInfos;
  };
}
