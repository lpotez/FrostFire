#include "SpriteRenderer.h"
#include "Engine/Shaders/techniques/GlobaleTechnique.h"
#include "Engine/Shaders/ShaderVariant.h"
#include "Engine/Texture.h"
#include "Engine/TextureManager.h"
#include "Engine/MeshManager.h"
#include "Engine/Utils/ErrorLogger.h"
#include "Engine/ECS/components/transform/TransformComponent.h"
#include "Engine/ECS/core/World.h"
#include <d3d11.h>
#include <DirectXMath.h>

using namespace FrostFireEngine;
using namespace DirectX;

struct MaterialData {
  XMFLOAT4 diffuseColor;
  XMFLOAT4 specularColor;
  XMFLOAT4 ambientColorMat;
};

static VertexLayoutDesc CreateLayout()
{
  VertexLayoutDesc layoutDesc;
  for (const auto& i : Vertex::layout) {
    layoutDesc.elements.push_back(i);
  }
  return layoutDesc;
}

SpriteRenderer::SpriteRenderer(ID3D11Device* device)
  : m_texture(nullptr), m_defaultSamplerState(nullptr), m_layout(CreateLayout()), m_opacity(1.0f),
    m_billboard(false)
{
  auto technique = std::make_unique<GlobaleTechnique>();
  SetTechnique(std::move(technique));
  InitializeConstantBuffers(device);

  D3D11_SAMPLER_DESC samplerDesc = {};
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
  if (FAILED(device->CreateSamplerState(&samplerDesc, &m_defaultSamplerState))) {
    ErrorLogger::Log("Failed to create sampler state in SpriteRenderer");
  }

  m_quadMesh = MeshManager::GetInstance().GetQuadMesh(device);
  SetOpaque(false);
}

SpriteRenderer::~SpriteRenderer()
{
  if (m_defaultSamplerState) m_defaultSamplerState->Release();
  m_defaultSamplerState = nullptr;
}

SpriteRenderer::SpriteRenderer(SpriteRenderer&& other) noexcept
  : BaseRendererComponent(std::move(other)),
    m_layout(std::move(other.m_layout)),
    m_defaultSamplerState(other.m_defaultSamplerState),
    m_texture(other.m_texture),
    m_quadMesh(std::move(other.m_quadMesh)),
    m_opacity(other.m_opacity),
    m_materialBuffer(std::move(other.m_materialBuffer)),
    m_billboard(other.m_billboard)
{
  other.m_defaultSamplerState = nullptr;
  other.m_texture = nullptr;
}

SpriteRenderer& SpriteRenderer::operator=(SpriteRenderer&& other) noexcept
{
  if (this != &other) {
    BaseRendererComponent::operator=(std::move(other));
    if (m_defaultSamplerState) m_defaultSamplerState->Release();
    m_layout = std::move(other.m_layout);
    m_defaultSamplerState = other.m_defaultSamplerState;
    m_texture = other.m_texture;
    m_quadMesh = std::move(other.m_quadMesh);
    m_opacity = other.m_opacity;
    m_materialBuffer = std::move(other.m_materialBuffer);
    m_billboard = other.m_billboard;
    other.m_defaultSamplerState = nullptr;
    other.m_texture = nullptr;
  }
  return *this;
}

void SpriteRenderer::Draw(ID3D11DeviceContext* deviceContext,
                          const XMMATRIX&      viewMatrix,
                          const XMMATRIX&      projectionMatrix,
                          RenderPass           currentPass)
{
  if (!IsVisible()) return;
  auto entity = World::GetInstance().GetEntity(GetOwner());
  if (!entity) return;
  auto transform = entity->GetComponent<TransformComponent>();
  if (!transform) return;
  XMMATRIX worldMatrix = transform->GetWorldMatrix();
  if (m_billboard) {
    XMVECTOR scale, rot, trans;
    XMMatrixDecompose(&scale, &rot, &trans, worldMatrix);
    XMMATRIX viewInv = XMMatrixInverse(nullptr, viewMatrix);
    XMVECTOR right = XMVector3Normalize(XMVectorSet(viewInv.r[0].m128_f32[0],
                                                    viewInv.r[0].m128_f32[1],
                                                    viewInv.r[0].m128_f32[2], 0.0f));
    XMVECTOR up = XMVector3Normalize(XMVectorSet(viewInv.r[1].m128_f32[0], viewInv.r[1].m128_f32[1],
                                                 viewInv.r[1].m128_f32[2], 0.0f));
    XMVECTOR forward = XMVector3Normalize(XMVectorSet(viewInv.r[2].m128_f32[0],
                                                      viewInv.r[2].m128_f32[1],
                                                      viewInv.r[2].m128_f32[2], 0.0f));
    XMMATRIX billboardMatrix;
    billboardMatrix.r[0] = XMVectorSet(XMVectorGetX(right), XMVectorGetY(right),
                                       XMVectorGetZ(right), 0.0f);
    billboardMatrix.r[1] = XMVectorSet(XMVectorGetX(up), XMVectorGetY(up), XMVectorGetZ(up), 0.0f);
    billboardMatrix.r[2] = XMVectorSet(XMVectorGetX(forward), XMVectorGetY(forward),
                                       XMVectorGetZ(forward), 0.0f);
    billboardMatrix.r[3] = XMVectorSet(XMVectorGetX(trans), XMVectorGetY(trans),
                                       XMVectorGetZ(trans), 1.0f);
    worldMatrix = XMMatrixScalingFromVector(scale) * billboardMatrix;

  }

  UpdateConstantBuffers(deviceContext, worldMatrix, viewMatrix, projectionMatrix);

  {
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(
      deviceContext->Map(m_materialBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
      MaterialData* matData = reinterpret_cast<MaterialData*>(mapped.pData);
      matData->diffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, m_opacity);
      matData->specularColor = XMFLOAT4(1, 1, 1, 32.0f);
      matData->ambientColorMat = XMFLOAT4(1, 1, 1, 0);
      deviceContext->Unmap(m_materialBuffer.Get(), 0);
    }
    deviceContext->PSSetConstantBuffers(1, 1, m_materialBuffer.GetAddressOf());
  }

  if (currentPass == RenderPass::Transparency || currentPass == RenderPass::UI) {
    const auto           features = GetActiveFeatures();
    const ShaderVariant* variant = GetTechnique()->GetVariantForPass(
      currentPass, features, GetVertexLayout());
    if (!variant) return;
    variant->Apply(deviceContext);
    auto&                     texManager = TextureManager::GetInstance();
    Texture*                  fallbackTex = texManager.GetTexture(L"__white_fallback__");
    ID3D11ShaderResourceView* srv = (m_texture && m_texture->GetShaderResourceView())
                                      ? m_texture->GetShaderResourceView().Get()
                                      : fallbackTex->GetShaderResourceView().Get();
    deviceContext->PSSetShaderResources(0, 1, &srv);
    deviceContext->PSSetSamplers(0, 1, &m_defaultSamplerState);
    if (m_quadMesh) m_quadMesh->Draw(deviceContext);
  }
}

const VertexLayoutDesc& SpriteRenderer::GetVertexLayout() const
{
  return m_layout;
}

ShaderTechnique* SpriteRenderer::GetTechnique() const
{
  return m_technique;
}

void SpriteRenderer::SetTexture(Texture* texture)
{
  m_texture = texture;
}

bool SpriteRenderer::InitializeConstantBuffers(ID3D11Device* device)
{
  if (!BaseRendererComponent::InitializeConstantBuffers(device)) return false;
  D3D11_BUFFER_DESC mbd = {};
  mbd.Usage = D3D11_USAGE_DYNAMIC;
  mbd.ByteWidth = sizeof(MaterialData);
  mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  mbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  if (FAILED(device->CreateBuffer(&mbd, nullptr, &m_materialBuffer))) {
    ErrorLogger::Log("Failed to create material buffer for SpriteRenderer.");
    return false;
  }
  return true;
}

void SpriteRenderer::SetOpacity(float opacity)
{
  m_opacity = opacity;
}

float SpriteRenderer::GetOpacity() const
{
  return m_opacity;
}

void SpriteRenderer::SetBillboard(bool enabled)
{
  m_billboard = enabled;
}

bool SpriteRenderer::IsBillboard() const
{
  return m_billboard;
}
