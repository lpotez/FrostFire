#include "ShaderVariant.h"
#include "Engine/Utils/ErrorLogger.h"
#include <sstream>
#include <d3dcompiler.h>
#include <algorithm>

namespace FrostFireEngine
{
  ShaderVariant::ShaderVariant() = default;

  ShaderVariant::~ShaderVariant()
  {
    Shutdown();
  }

  bool ShaderVariant::Initialize(ID3D11Device*                             device,
                                 const std::wstring&                       shaderPath,
                                 const std::vector<std::string>&           featureIds,
                                 const std::map<std::string, std::string>& defines,
                                 const VertexLayoutDesc&                   layout,
                                 const std::string&                        vsEntry,
                                 const std::string&                        psEntry)
  {
    m_definesList = defines;
    m_activeFeatures = featureIds;

    {
      std::stringstream keyStream;
      for (const auto& id : featureIds) {
        keyStream << id << "_";
      }
      m_variantKey = keyStream.str();
    }

    ID3DBlob* vertexShaderBlob = nullptr;
    ID3DBlob* pixelShaderBlob = nullptr;

    // Compile vertex shader
    if (!CompileShaderFromFile(shaderPath, vsEntry, "vs_5_0", defines, &vertexShaderBlob)) {
      ErrorLogger::Log("Failed to compile vertex shader: " + std::string(vsEntry));
      return false;
    }

    // Compile pixel shader
    if (!CompileShaderFromFile(shaderPath, psEntry, "ps_5_0", defines, &pixelShaderBlob)) {
      ErrorLogger::Log("Failed to compile pixel shader: " + std::string(psEntry));
      if (vertexShaderBlob) vertexShaderBlob->Release();
      return false;
    }

    // Create the vertex shader
    HRESULT result = device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(),
                                                vertexShaderBlob->GetBufferSize(),
                                                nullptr,
                                                &m_vertexShader);
    if (FAILED(result)) {
      ErrorLogger::Log("Failed to create vertex shader.");
      vertexShaderBlob->Release();
      pixelShaderBlob->Release();
      return false;
    }

    // Create the pixel shader
    result = device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(),
                                       pixelShaderBlob->GetBufferSize(),
                                       nullptr,
                                       &m_pixelShader);
    if (FAILED(result)) {
      ErrorLogger::Log("Failed to create pixel shader.");
      vertexShaderBlob->Release();
      pixelShaderBlob->Release();
      return false;
    }

    // Create input layout
    if (!CreateInputLayout(device, vertexShaderBlob, layout)) {
      ErrorLogger::Log("Failed to create input layout.");
      vertexShaderBlob->Release();
      pixelShaderBlob->Release();
      return false;
    }

    // Create sampler state
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    result = device->CreateSamplerState(&samplerDesc, &m_samplerState);
    if (FAILED(result)) {
      ErrorLogger::Log("Failed to create sampler state.");
      vertexShaderBlob->Release();
      pixelShaderBlob->Release();
      return false;
    }

    vertexShaderBlob->Release();
    pixelShaderBlob->Release();

    return true;
  }

  void ShaderVariant::Shutdown()
  {
    if (m_samplerState) {
      m_samplerState->Release();
      m_samplerState = nullptr;
    }
    if (m_matrixBuffer) {
      m_matrixBuffer->Release();
      m_matrixBuffer = nullptr;
    }
    if (m_inputLayout) {
      m_inputLayout->Release();
      m_inputLayout = nullptr;
    }
    if (m_pixelShader) {
      m_pixelShader->Release();
      m_pixelShader = nullptr;
    }
    if (m_vertexShader) {
      m_vertexShader->Release();
      m_vertexShader = nullptr;
    }
  }

  bool ShaderVariant::Apply(ID3D11DeviceContext* deviceContext) const
  {
    if (!deviceContext || !m_vertexShader || !m_pixelShader) {
      return false;
    }

    deviceContext->IASetInputLayout(m_inputLayout);
    deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
    deviceContext->PSSetShader(m_pixelShader, nullptr, 0);
    deviceContext->PSSetSamplers(0, 1, &m_samplerState);

    return true;
  }


  std::string ShaderVariant::GetVariantKey() const
  {
    return m_variantKey;
  }

  bool ShaderVariant::HasFeature(const std::string& id) const
  {
    return std::find(m_activeFeatures.begin(), m_activeFeatures.end(), id) != m_activeFeatures.
    end();
  }

  bool ShaderVariant::CompileShaderFromFile(const std::wstring&                       shaderPath,
                                            const std::string&                        entryPoint,
                                            const std::string&                        shaderModel,
                                            const std::map<std::string, std::string>& defines,
                                            ID3DBlob**                                shaderBlob)
  {
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    std::vector<D3D_SHADER_MACRO> macros;
    for (const auto& def : defines) {
      D3D_SHADER_MACRO m;
      m.Name = def.first.c_str();
      m.Definition = def.second.c_str();
      macros.push_back(m);
    }
    D3D_SHADER_MACRO nullMacro = {nullptr, nullptr};
    macros.push_back(nullMacro);

    ID3DBlob* errorBlob = nullptr;
    HRESULT   hr = D3DCompileFromFile(shaderPath.c_str(),
                                    macros.data(),
                                    D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                    entryPoint.c_str(),
                                    shaderModel.c_str(),
                                    flags,
                                    0,
                                    shaderBlob,
                                    &errorBlob);

    if (FAILED(hr)) {
      if (errorBlob) {
        ErrorLogger::Log(static_cast<char*>(errorBlob->GetBufferPointer()));
        errorBlob->Release();
      }
      return false;
    }

    if (errorBlob) errorBlob->Release();
    return true;
  }

  bool ShaderVariant::CreateInputLayout(ID3D11Device*           device,
                                        ID3DBlob*               vertexShaderBlob,
                                        const VertexLayoutDesc& layout)
  {
    if (layout.elements.empty()) {
      // Aucune entrée nécessaire pour ce shader, on n'appelle pas CreateInputLayout
      m_inputLayout = nullptr;
      return true;
    }
    HRESULT result = device->CreateInputLayout(layout.elements.data(),
                                               static_cast<UINT>(layout.elements.size()),
                                               vertexShaderBlob->GetBufferPointer(),
                                               vertexShaderBlob->GetBufferSize(),
                                               &m_inputLayout);
    if (FAILED(result)) {
      ErrorLogger::Log("Failed to create input layout.");
      return false;
    }
    return true;
  }
}
