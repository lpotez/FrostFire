#include "RenderShader.h"
#include "Engine/Utils/ErrorLogger.h"

namespace FrostFireEngine
{
  RenderShader::RenderShader(ID3D11Device* device) :
    m_device(device),
    m_variantManager(std::make_unique<ShaderVariantManager>()),
    m_variantCache(std::make_unique<VariantCache>())
  {
  }

  RenderShader::~RenderShader()
  {
    Shutdown();
  }

  bool RenderShader::Initialize(const std::wstring& shaderPath)
  {
    m_shaderPath = shaderPath;
    return true;
  }

  void RenderShader::Shutdown()
  {
    m_variants.clear();
    m_variantCache.reset();
    m_variantManager.reset();
  }

  ShaderVariant* RenderShader::GetVariant(const std::vector<std::string>& featureIds,
                                          const VertexLayoutDesc&         layout) const
  {
    if (!m_variantManager->ValidateFeatureCombination(featureIds)) {
      return nullptr;
    }

    std::map<std::string, std::string> defines;
    for (const auto& id : featureIds) {
      defines["USE_" + id] = "1";
    }

    return m_variantCache->GetOrCreateVariant(m_device, m_shaderPath, featureIds, defines, layout);
  }
}
