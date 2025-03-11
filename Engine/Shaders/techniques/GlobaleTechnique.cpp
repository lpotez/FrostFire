#include "GlobaleTechnique.h"
#include "Engine/Shaders/features/RenderPass.h"

namespace FrostFireEngine
{
  GlobaleTechnique::GlobaleTechnique()
  {
    // Passe GBuffer
    {
      PassShaderInfo info;
      info.shaderFile = L"Assets/Shaders/GBufferPass.fx";
      info.vsEntryPoint = "VS";
      info.psEntryPoint = "PS_GBuffer";
      m_passInfos[RenderPass::GBuffer] = info;
    }

    // Passe Lighting
    {
      PassShaderInfo info;
      info.shaderFile = L"Assets/Shaders/LightingPass.fx";
      info.vsEntryPoint = "VS"; // fullscreen triangle
      info.psEntryPoint = "PS_Lighting";
      m_passInfos[RenderPass::Lighting] = info;
    }

    // Passe Transparency
    {
      PassShaderInfo info;
      info.shaderFile = L"Assets/Shaders/TransparencyPass.fx";
      info.vsEntryPoint = "VS_Transparency";
      info.psEntryPoint = "PS_Transparency";
      m_passInfos[RenderPass::Transparency] = info;
    }

    // Passe Skybox
    {
      PassShaderInfo info;
      info.shaderFile = L"Assets/Shaders/SkyboxPass.fx";
      info.vsEntryPoint = "VS_Skybox";
      info.psEntryPoint = "PS_Skybox";
      m_passInfos[RenderPass::Skybox] = info;
    }

    // Passe Final
    {
      PassShaderInfo info;
      info.shaderFile = L"Assets/Shaders/FinalPass.fx";
      info.vsEntryPoint = "VS";
      info.psEntryPoint = "PS_Final";
      m_passInfos[RenderPass::Final] = info;
    }

    // Passe Shadow
    {
      PassShaderInfo info;
      info.shaderFile = L"Assets/Shaders/ShadowPass.fx";
      info.vsEntryPoint = "VS_Shadow";
      info.psEntryPoint = "PS_Shadow";
      m_passInfos[RenderPass::Shadow] = info;
    }

    // Passe UI
    {
      PassShaderInfo info;
      info.shaderFile = L"Assets/Shaders/UIPass.fx";
      info.vsEntryPoint = "VS_UI";
      info.psEntryPoint = "PS_UI";
      m_passInfos[RenderPass::UI] = info;
    }

    // Passe Debug
    {
      PassShaderInfo info;
      info.shaderFile = L"Assets/Shaders/Debug.fx";
      info.vsEntryPoint = "VS_Debug";
      info.psEntryPoint = "PS_Debug";
      m_passInfos[RenderPass::Debug] = info;
    }

    // Passe PostProcess
    {
      PassShaderInfo info;
      info.shaderFile = L"Assets/Shaders/postprocess.fx";
      info.vsEntryPoint = "VS_PostProcess";
      info.psEntryPoint = "PS_Vignette";
      m_passInfos[RenderPass::PostProcess] = info;
    }
  }

  bool GlobaleTechnique::GetPassShaderInfo(RenderPass pass, PassShaderInfo& outInfo) const
  {
    auto it = m_passInfos.find(pass);
    if (it == m_passInfos.end()) return false;
    outInfo = it->second;
    return true;
  }
}
