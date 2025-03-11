#include "PBRRenderer.h"
#include "Engine/ECS/core/World.h"
#include "Engine/ECS/components/transform/TransformComponent.h"
#include "Engine/ECS/components/mesh/MeshComponent.h"
#include "Engine/Shaders/techniques/GlobaleTechnique.h"
#include "Engine/Shaders/ShaderVariant.h"
#include "Engine/Texture.h"
#include "Engine/TextureManager.h"
#include "Engine/Utils/ErrorLogger.h"
#include "Engine/MeshManager.h"
#include <assert.h>

#include "Engine/Shaders/features/PBRFeature.h"

using namespace FrostFireEngine;
using namespace DirectX;

REGISTER_INHERITANCE(PBRRenderer, BaseRendererComponent)


static VertexLayoutDesc CreateLayout()
{
  VertexLayoutDesc layoutDesc;
  for (const auto& i : Vertex::layout) {
    layoutDesc.elements.push_back(i);
  }
  return layoutDesc;
}

PBRRenderer::PBRRenderer(ID3D11Device* device)
  : m_layout(CreateLayout())
{
  m_world = &World::GetInstance();

  auto technique = std::make_unique<GlobaleTechnique>();
  SetTechnique(std::move(technique));

  auto feature = std::make_unique<PBRFeature>(device);
  AddFeature(std::move(feature));

  PBRRenderer::InitializeConstantBuffers(device);

  D3D11_SAMPLER_DESC samplerDesc = {};
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  HRESULT hr = device->CreateSamplerState(&samplerDesc, &m_defaultSamplerState);
  if (FAILED(hr)) {
    ErrorLogger::Log("Failed to create default sampler state in PBRRenderer");
  }
}

PBRRenderer::~PBRRenderer()
{
  if (m_defaultSamplerState) {
    m_defaultSamplerState->Release();
    m_defaultSamplerState = nullptr;
  }
}

PBRRenderer::PBRRenderer(PBRRenderer&& other) noexcept
  : BaseRendererComponent(std::move(other)),
    m_layout(std::move(other.m_layout))
{
  m_technique = std::move(other.m_technique);
  m_defaultSamplerState = other.m_defaultSamplerState;
  m_albedoTexture = other.m_albedoTexture;
  m_normalMap = other.m_normalMap;
  m_metallicRoughnessMap = other.m_metallicRoughnessMap;
  m_aoMap = other.m_aoMap;

  other.m_defaultSamplerState = nullptr;
  other.m_albedoTexture = nullptr;
  other.m_normalMap = nullptr;
  other.m_metallicRoughnessMap = nullptr;
  other.m_aoMap = nullptr;
}

PBRRenderer& PBRRenderer::operator=(PBRRenderer&& other) noexcept
{
  if (this != &other) {
    BaseRendererComponent::operator=(std::move(other));
    m_technique = std::move(other.m_technique);
    m_layout = std::move(other.m_layout);

    if (m_defaultSamplerState) m_defaultSamplerState->Release();

    m_defaultSamplerState = other.m_defaultSamplerState;
    m_albedoTexture = other.m_albedoTexture;
    m_normalMap = other.m_normalMap;
    m_metallicRoughnessMap = other.m_metallicRoughnessMap;
    m_aoMap = other.m_aoMap;

    other.m_defaultSamplerState = nullptr;
    other.m_albedoTexture = nullptr;
    other.m_normalMap = nullptr;
    other.m_metallicRoughnessMap = nullptr;
    other.m_aoMap = nullptr;
  }
  return *this;
}

void PBRRenderer::Draw(ID3D11DeviceContext* deviceContext,
                       const XMMATRIX&      viewMatrix,
                       const XMMATRIX&      projectionMatrix,
                       RenderPass           currentPass)
{
  if (!IsVisible()) return;
  const auto entity = World::GetInstance().GetEntity(GetOwner());
  if (!entity) return;

  const auto transform = entity->GetComponent<TransformComponent>();
  const auto meshComponent = entity->GetComponent<MeshComponent>();

  if (!transform || !meshComponent) return;

  const XMMATRIX worldMatrix = transform->GetWorldMatrix();
  UpdateConstantBuffers(deviceContext, worldMatrix, viewMatrix, projectionMatrix);

  if (currentPass == RenderPass::GBuffer || currentPass == RenderPass::Transparency) {
    const auto           features = GetActiveFeatures();
    const ShaderVariant* variant = GetTechnique()->GetVariantForPass(
      currentPass, features, GetVertexLayout());
    if (!variant) {
      ErrorLogger::Log("No shader variant found for current pass in PBRRenderer.");
      return;
    }

    for (const auto& f : m_features) {
      f->UpdateParameters(deviceContext);
    }

    variant->Apply(deviceContext);

    auto&    texManager = TextureManager::GetInstance();
    Texture* fallbackTex = texManager.GetTexture(L"__white_fallback__");
    Texture* normalFallbackTex = texManager.GetTexture(L"__neutralnormal_fallback__");
    assert(fallbackTex && "Fallback texture not found!");
    assert(normalFallbackTex && "Neutral normal fallback texture not found!");

    ID3D11ShaderResourceView* albedoSRV = (m_albedoTexture && m_albedoTexture->
                                            GetShaderResourceView())
                                            ? m_albedoTexture->GetShaderResourceView().Get()
                                            : fallbackTex->GetShaderResourceView().Get();

    ID3D11ShaderResourceView* normalSRV = (m_normalMap && m_normalMap->GetShaderResourceView())
                                            ? m_normalMap->GetShaderResourceView().Get()
                                            : normalFallbackTex->GetShaderResourceView().Get();

    ID3D11ShaderResourceView* mrSRV = (m_metallicRoughnessMap && m_metallicRoughnessMap->
                                        GetShaderResourceView())
                                        ? m_metallicRoughnessMap->GetShaderResourceView().Get()
                                        : fallbackTex->GetShaderResourceView().Get();

    ID3D11ShaderResourceView* aoSRV = (m_aoMap && m_aoMap->GetShaderResourceView())
                                        ? m_aoMap->GetShaderResourceView().Get()
                                        : fallbackTex->GetShaderResourceView().Get();

    ID3D11ShaderResourceView *irradianceSRV = (m_irradianceMap && m_irradianceMap->GetShaderResourceView())
      ? m_irradianceMap->GetShaderResourceView().Get()
      : fallbackTex->GetShaderResourceView().Get();

    ID3D11ShaderResourceView *prefilteredEnvSRV = (m_prefilteredEnvMap && m_prefilteredEnvMap->GetShaderResourceView())
      ? m_prefilteredEnvMap->GetShaderResourceView().Get()
      : fallbackTex->GetShaderResourceView().Get();

    ID3D11ShaderResourceView *brdfSRV = (m_brdfLUT && m_brdfLUT->GetShaderResourceView())
      ? m_brdfLUT->GetShaderResourceView().Get()
      : fallbackTex->GetShaderResourceView().Get();

    ID3D11ShaderResourceView* srvs[4] = {albedoSRV, normalSRV, mrSRV, aoSRV};
    deviceContext->PSSetShaderResources(0, 4, srvs);

    deviceContext->PSSetShaderResources(4, 1, &irradianceSRV);
    deviceContext->PSSetShaderResources(5, 1, &prefilteredEnvSRV);
    deviceContext->PSSetShaderResources(6, 1, &brdfSRV);

    deviceContext->PSSetSamplers(0, 1, &m_defaultSamplerState);
  }

  if (const auto mesh = meshComponent->GetMesh()) {
    mesh->Draw(deviceContext);
  }
}

const VertexLayoutDesc& PBRRenderer::GetVertexLayout() const
{
  return m_layout;
}

ShaderTechnique* PBRRenderer::GetTechnique() const
{
  return m_technique;
}

bool PBRRenderer::InitializeConstantBuffers(ID3D11Device* device)
{
  return BaseRendererComponent::InitializeConstantBuffers(device);
}

// Setters/Getters PBR
void PBRRenderer::SetBaseColor(const XMFLOAT4& color) const
{
  for (auto& f : m_features) {
    if (auto pf = dynamic_cast<PBRFeature*>(f.get())) {
      pf->SetBaseColor(color);
    }
  }
}
void PBRRenderer::SetMetallic(float metallic) const
{
  for (auto& f : m_features) {
    if (auto pf = dynamic_cast<PBRFeature*>(f.get())) {
      pf->SetMetallic(metallic);
    }
  }
}
void PBRRenderer::SetRoughness(float roughness) const
{
  for (auto& f : m_features) {
    if (auto pf = dynamic_cast<PBRFeature*>(f.get())) {
      pf->SetRoughness(roughness);
    }
  }
}
void PBRRenderer::SetAmbientOcclusion(float ao) const
{
  for (auto& f : m_features) {
    if (auto pf = dynamic_cast<PBRFeature*>(f.get())) {
      pf->SetAmbientOcclusion(ao);
    }
  }
}

XMFLOAT4 PBRRenderer::GetBaseColor() const
{
  for (auto& f : m_features) {
    if (auto pf = dynamic_cast<PBRFeature*>(f.get())) {
      return pf->GetBaseColor();
    }
  }
  return {1, 1, 1, 1};
}

float PBRRenderer::GetMetallic() const
{
  for (auto& f : m_features) {
    if (auto pf = dynamic_cast<PBRFeature*>(f.get())) {
      return pf->GetMetallic();
    }
  }
  return 0.0f;
}

float PBRRenderer::GetRoughness() const
{
  for (auto& f : m_features) {
    if (auto pf = dynamic_cast<PBRFeature*>(f.get())) {
      return pf->GetRoughness();
    }
  }
  return 0.5f;
}

float PBRRenderer::GetAO() const
{
  for (auto& f : m_features) {
    if (auto pf = dynamic_cast<PBRFeature*>(f.get())) {
      return pf->GetAO();
    }
  }
  return 1.0f;
}
