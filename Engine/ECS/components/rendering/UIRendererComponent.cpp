#include "UIRendererComponent.h"
#include "UIBaseRendererComponent.h"
#include "Engine/ECS/core/World.h"
#include "Engine/ECS/components/transform/TransformComponent.h"
#include "Engine/ECS/components/transform/RectTransformComponent.h"
#include "Engine/Texture.h"
#include "Engine/TextureManager.h"
#include "Engine/Shaders/techniques/GlobaleTechnique.h"
#include "Engine/Shaders/ShaderVariant.h"
#include "Engine/Utils/ErrorLogger.h"

using namespace FrostFireEngine;
using namespace DirectX;

REGISTER_INHERITANCE(UIRendererComponent, UIBaseRendererComponent)

VertexLayoutDesc UIRendererComponent::CreateUILayout()
{
  VertexLayoutDesc         layoutDesc;
  D3D11_INPUT_ELEMENT_DESC posElement = {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
    D3D11_INPUT_PER_VERTEX_DATA, 0};
  D3D11_INPUT_ELEMENT_DESC uvElement = {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8,
    D3D11_INPUT_PER_VERTEX_DATA, 0};

  layoutDesc.elements.push_back(posElement);
  layoutDesc.elements.push_back(uvElement);
  return layoutDesc;
}

UIRendererComponent::UIRendererComponent(const DispositifD3D11* dispositif)
  : m_dispositif(dispositif)
{
  m_layout = CreateUILayout();
  auto technique = std::make_unique<GlobaleTechnique>();
  SetTechnique(std::move(technique));

  UIRendererComponent::InitializeConstantBuffers(dispositif->GetD3DDevice());
  CreateQuadGeometry(dispositif->GetD3DDevice());
}

UIRendererComponent::~UIRendererComponent() = default;

bool UIRendererComponent::InitializeConstantBuffers(ID3D11Device* device)
{
  // Appel à la classe de base UIBaseRendererComponent qui gère déjà colorBuffer + sampler
  return UIBaseRendererComponent::InitializeConstantBuffers(device);
}

void UIRendererComponent::CreateQuadGeometry(ID3D11Device* device)
{
  UIVertex vertices[4] = {
    {{0.0f, 0.0f}, {0.0f, 0.0f}},
    {{1.0f, 0.0f}, {1.0f, 0.0f}},
    {{1.0f, 1.0f}, {1.0f, 1.0f}},
    {{0.0f, 1.0f}, {0.0f, 1.0f}}
  };

  D3D11_BUFFER_DESC vbd = {};
  vbd.Usage = D3D11_USAGE_DEFAULT;
  vbd.ByteWidth = static_cast<UINT>(sizeof(vertices));
  vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

  D3D11_SUBRESOURCE_DATA initData = {};
  initData.pSysMem = vertices;

  if (FAILED(device->CreateBuffer(&vbd, &initData, &m_vertexBuffer))) {
    ErrorLogger::Log("UIRendererComponent: Failed to create vertex buffer.");
  }

  DWORD             indices[6] = {0, 1, 2, 0, 2, 3};
  D3D11_BUFFER_DESC ibd = {};
  ibd.Usage = D3D11_USAGE_DEFAULT;
  ibd.ByteWidth = sizeof(indices);
  ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

  initData.pSysMem = indices;
  if (FAILED(device->CreateBuffer(&ibd, &initData, &m_indexBuffer))) {
    ErrorLogger::Log("UIRendererComponent: Failed to create index buffer.");
  }
}

const VertexLayoutDesc& UIRendererComponent::GetVertexLayout() const
{
  return m_layout;
}

ShaderTechnique* UIRendererComponent::GetTechnique() const
{
  return m_technique;
}

void UIRendererComponent::SetTexture(Texture* texture)
{
  m_texture = texture;
  if (m_texture) {
    m_contentWidth = static_cast<float>(texture->GetWidth());
    m_contentHeight = static_cast<float>(texture->GetHeight());
  }
  else {
    m_contentWidth = 0.0f;
    m_contentHeight = 0.0f;
  }
}

void UIRendererComponent::Draw(ID3D11DeviceContext* deviceContext,
                               const XMMATRIX&      viewMatrix,
                               const XMMATRIX&      projectionMatrix,
                               RenderPass           currentPass)
{
  if (!IsVisible()) return;

  auto entity = World::GetInstance().GetEntity(GetOwner());
  if (!entity) return;

  auto transform = entity->GetComponent<TransformComponent>();
  auto rectTransform = entity->GetComponent<RectTransformComponent>();
  if (!transform || !rectTransform) return;

  float viewportWidth = m_dispositif->GetViewportWidth();
  float viewportHeight = m_dispositif->GetViewportHeight();

  XMMATRIX uiMatrix = rectTransform->GetUITransformMatrix(transform,
                                                          m_contentWidth,
                                                          m_contentHeight);


  XMMATRIX world = uiMatrix;
  XMMATRIX view = XMMatrixIdentity();
  XMMATRIX proj = XMMatrixOrthographicOffCenterLH(0.0f, viewportWidth, viewportHeight, 0.0f, 0.0f,
                                                  1.0f);

  UpdateCommonConstantBuffers(deviceContext, world, view, proj);

  auto features = GetActiveFeatures();
  auto variant = GetTechnique()->GetVariantForPass(currentPass, features, GetVertexLayout());
  if (!variant) {
    ErrorLogger::Log("UIRendererComponent: No UI shader variant found.");
    return;
  }

  variant->Apply(deviceContext);

  auto&                     texManager = TextureManager::GetInstance();
  Texture*                  fallbackTex = texManager.GetTexture(L"__white_fallback__");
  ID3D11ShaderResourceView* srv = (m_texture && m_texture->GetShaderResourceView())
                                    ? m_texture->GetShaderResourceView().Get()
                                    : fallbackTex->GetShaderResourceView().Get();

  deviceContext->PSSetShaderResources(0, 1, &srv);

  ApplyCommonUISettings(deviceContext);

  UINT stride = sizeof(UIVertex);
  UINT offset = 0;
  deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
  deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
  deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  deviceContext->DrawIndexed(6, 0, 0);
}
