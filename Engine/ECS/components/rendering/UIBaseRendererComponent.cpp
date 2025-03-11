#include "UIBaseRendererComponent.h"
#include "Engine/Utils/ErrorLogger.h"

struct UIColorCBuffer {
  XMFLOAT4 color;
};

using namespace FrostFireEngine;

bool UIBaseRendererComponent::InitializeConstantBuffers(ID3D11Device* device)
{
  if (!BaseRendererComponent::InitializeConstantBuffers(device)) return false;

  D3D11_BUFFER_DESC cbd = {};
  cbd.Usage = D3D11_USAGE_DEFAULT;
  cbd.ByteWidth = sizeof(UIColorCBuffer);
  cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

  if (FAILED(device->CreateBuffer(&cbd, nullptr, &m_colorBuffer))) {
    ErrorLogger::Log("UIBaseRendererComponent: Failed to create color constant buffer.");
    return false;
  }

  D3D11_SAMPLER_DESC samplerDesc = {};
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

  if (FAILED(device->CreateSamplerState(&samplerDesc, &m_samplerState))) {
    ErrorLogger::Log("UIBaseRendererComponent: Failed to create sampler state.");
    return false;
  }

  return true;
}

void UIBaseRendererComponent::UpdateCommonConstantBuffers(ID3D11DeviceContext* deviceContext,
                                                          const XMMATRIX&      worldMatrix,
                                                          const XMMATRIX&      viewMatrix,
                                                          const XMMATRIX&      projMatrix) const
{
  UpdateConstantBuffers(deviceContext, worldMatrix, viewMatrix, projMatrix);

  UIColorCBuffer data;
  data.color = m_color;
  deviceContext->UpdateSubresource(m_colorBuffer.Get(), 0, nullptr, &data, 0, 0);
}

void UIBaseRendererComponent::ApplyCommonUISettings(ID3D11DeviceContext* deviceContext)
{
  deviceContext->PSSetConstantBuffers(1, 1, m_colorBuffer.GetAddressOf());
  deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
}
