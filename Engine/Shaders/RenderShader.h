#pragma once

#include <string>
#include <memory>
#include <d3d11.h>
#include <unordered_map>

#include "ShaderVariant.h"
#include "ShaderVariantManager.h"
#include "VariantCache.h"

namespace FrostFireEngine
{
  class RenderShader {
  public:
    RenderShader(ID3D11Device* device);
    ~RenderShader();

    bool Initialize(const std::wstring& shaderPath);
    void Shutdown();

    ShaderVariant* GetVariant(const std::vector<std::string>& featureIds, const VertexLayoutDesc& layout) const;
    const std::wstring& GetShaderPath() const
    {
      return m_shaderPath;
    }

  private:
    ID3D11Device*                                                   m_device;
    std::wstring                                                    m_shaderPath;
    std::unique_ptr<ShaderVariantManager>                           m_variantManager;
    std::unique_ptr<VariantCache>                                   m_variantCache;
    std::unordered_map<std::string, std::unique_ptr<ShaderVariant>> m_variants;
  };
}
