#include "TextTechnique.h"

namespace FrostFireEngine
{
  TextTechnique::TextTechnique()
  {
    PassShaderInfo info;
    info.shaderFile = L"Assets/Shaders/TextPass.fx";
    info.vsEntryPoint = "VS_Text";
    info.psEntryPoint = "PS_Text";
    m_passInfos[RenderPass::UI] = info;
  }

  bool TextTechnique::GetPassShaderInfo(RenderPass pass, PassShaderInfo& outInfo) const
  {
    auto it = m_passInfos.find(pass);
    if (it != m_passInfos.end()) {
      outInfo = it->second;
      return true;
    }
    return false;
  }
}
