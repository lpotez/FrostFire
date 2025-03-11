#pragma once
#include <d3d11.h>
#include <string>
#include <map>
#include <vector>
#include <wrl/client.h>

#include "Engine/Shaders/VertexLayoutDesc.h"

namespace FrostFireEngine
{
  class ShaderVariant {
  public:
    ShaderVariant();
    ~ShaderVariant();

    // Ajout de vsEntry et psEntry dans les paramètres
    bool Initialize(ID3D11Device*                             device,
                    const std::wstring&                       shaderPath,
                    const std::vector<std::string>&           featureIds,
                    const std::map<std::string, std::string>& defines,
                    const VertexLayoutDesc&                   layout,
                    const std::string&                        vsEntry = "VS",
                    const std::string&                        psEntry = "PS");

    void        Shutdown();
    bool        Apply(ID3D11DeviceContext* deviceContext) const;
    std::string GetVariantKey() const;
    bool        HasFeature(const std::string& id) const;

  private:
    static bool CompileShaderFromFile(const std::wstring&                       shaderPath,
                                      const std::string&                        entryPoint,
                                      const std::string&                        shaderModel,
                                      const std::map<std::string, std::string>& defines,
                                      ID3DBlob**                                shaderBlob);

    bool CreateInputLayout(ID3D11Device*           device,
                           ID3DBlob*               vertexShaderBlob,
                           const VertexLayoutDesc& layout);

    std::string                        m_variantKey;
    std::map<std::string, std::string> m_definesList;
    std::vector<std::string>           m_activeFeatures;

    ID3D11VertexShader* m_vertexShader = nullptr;
    ID3D11PixelShader*  m_pixelShader = nullptr;
    ID3D11InputLayout*  m_inputLayout = nullptr;
    ID3D11Buffer*       m_matrixBuffer = nullptr;
    ID3D11SamplerState* m_samplerState = nullptr;
  };
}
