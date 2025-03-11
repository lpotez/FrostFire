#include "PBRFeature.h"
#include "Engine/Shaders/features/RenderPass.h"
#include "Engine/Utils/ErrorLogger.h"

using namespace FrostFireEngine;

PBRFeature::PBRFeature(ID3D11Device* device)
  : m_materialBuffer(nullptr)
{
  // Valeurs PBR par défaut
  m_params.baseColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
  m_params.metallic = 0.0f;
  m_params.roughness = 0.5f;
  m_params.ao = 1.0f;
  m_params.padding = 0.0f;

  InitializeBuffers(device);
}

PBRFeature::~PBRFeature()
{
  if (m_materialBuffer) {
    m_materialBuffer->Release();
    m_materialBuffer = nullptr;
  }
}

std::vector<std::string> PBRFeature::GetDefines() const
{
  // Indique aux shaders que le PBR est actif
  return {"USE_PBR"};
}

void PBRFeature::UpdateParameters(ID3D11DeviceContext* context)
{
  if (!context || !m_materialBuffer) return;

  D3D11_MAPPED_SUBRESOURCE mappedResource;
  HRESULT hr = context->Map(m_materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(hr)) {
    ErrorLogger::Log("Failed to map PBR material buffer");
    return;
  }

  memcpy(mappedResource.pData, &m_params, sizeof(PBRMaterialParams));
  context->Unmap(m_materialBuffer, 0);

  context->PSSetConstantBuffers(1, 1, &m_materialBuffer);
}

std::string PBRFeature::GetId() const
{
  return "PBR";
}

FeatureMetadata PBRFeature::GetMetadata() const
{
  FeatureMetadata metadata;
  // Compatible avec GBuffer, Lighting, Transparency
  metadata.renderPassMask = static_cast<uint32_t>(RenderPass::GBuffer) |
  static_cast<uint32_t>(RenderPass::Lighting) |
  static_cast<uint32_t>(RenderPass::Transparency);
  return metadata;
}

uint32_t PBRFeature::GetFeatureGroup() const
{
  return 0;
}

void PBRFeature::SetBaseColor(const DirectX::XMFLOAT4& color)
{
  m_params.baseColor = color;
}
void PBRFeature::SetMetallic(float metallic)
{
  m_params.metallic = metallic;
}
void PBRFeature::SetRoughness(float roughness)
{
  m_params.roughness = roughness;
}
void PBRFeature::SetAmbientOcclusion(float ao)
{
  m_params.ao = ao;
}

bool PBRFeature::InitializeBuffers(ID3D11Device* device)
{
  if (!device) return false;

  D3D11_BUFFER_DESC bd = {};
  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.ByteWidth = sizeof(PBRMaterialParams);
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  D3D11_SUBRESOURCE_DATA initData = {};
  initData.pSysMem = &m_params;

  HRESULT hr = device->CreateBuffer(&bd, &initData, &m_materialBuffer);
  if (FAILED(hr)) {
    ErrorLogger::Log("Failed to create PBR material buffer");
    return false;
  }
  return true;
}
