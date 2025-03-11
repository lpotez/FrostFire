#include "BaseRendererComponent.h"
#include "Engine/Utils/ErrorLogger.h"
#include <d3d11.h>

#include "Engine/ECS/core/World.h"
#include "Engine/ECS/components/transform/TransformComponent.h"

namespace FrostFireEngine
{
  class TransformComponent;
}

using namespace FrostFireEngine;

BaseRendererComponent::BaseRendererComponent()
  : m_technique(nullptr), m_matrixBuffer(nullptr), m_visible(true), m_opaque(true)
{
}

BaseRendererComponent::~BaseRendererComponent()
{
  if (m_matrixBuffer) {
    m_matrixBuffer->Release();
    m_matrixBuffer = nullptr;
  }
  m_technique = nullptr;
}

BaseRendererComponent::BaseRendererComponent(BaseRendererComponent&& other) noexcept
  : Component(std::move(other)),
    m_features(std::move(other.m_features)),
    m_technique(other.m_technique),
    m_ownedTechnique(std::move(other.m_ownedTechnique)),
    m_matrixBuffer(other.m_matrixBuffer),
    m_visible(other.m_visible),
    m_opaque(other.m_opaque)
{
  other.m_technique = nullptr;
  other.m_matrixBuffer = nullptr;
}

BaseRendererComponent& BaseRendererComponent::operator=(BaseRendererComponent&& other) noexcept
{
  if (this != &other) {
    Component::operator=(std::move(other));

    // Libérer les ressources actuelles
    if (m_matrixBuffer) {
      m_matrixBuffer->Release();
    }

    m_features = std::move(other.m_features);
    m_technique = other.m_technique;
    m_ownedTechnique = std::move(other.m_ownedTechnique);
    m_matrixBuffer = other.m_matrixBuffer;
    m_visible = other.m_visible;
    m_opaque = other.m_opaque;

    other.m_technique = nullptr;
    other.m_matrixBuffer = nullptr;
  }
  return *this;
}

void BaseRendererComponent::AddFeature(std::unique_ptr<BaseShaderFeature> feature)
{
  m_features.push_back(std::move(feature));
}

void BaseRendererComponent::SetTechnique(std::unique_ptr<ShaderTechnique> technique)
{
  m_ownedTechnique = std::move(technique);
  m_technique = m_ownedTechnique.get();
}

bool BaseRendererComponent::InitializeConstantBuffers(ID3D11Device* device)
{
  if (!device) {
    ErrorLogger::Log("Invalid device pointer in InitializeConstantBuffers.");
    return false;
  }

  D3D11_BUFFER_DESC matrixBufferDesc = {};
  matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  matrixBufferDesc.ByteWidth = sizeof(TransformMatrixBuffer);
  matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  HRESULT result = device->CreateBuffer(&matrixBufferDesc, nullptr, &m_matrixBuffer);
  if (FAILED(result)) {
    ErrorLogger::Log("Failed to create constant buffer.");
    return false;
  }

  return true;
}

bool BaseRendererComponent::UpdateConstantBuffers(
  ID3D11DeviceContext* deviceContext,
  const XMMATRIX&      worldMatrix,
  const XMMATRIX&      viewMatrix,
  const XMMATRIX&      projMatrix) const
{
  if (!deviceContext || !m_matrixBuffer) return false;

  TransformMatrixBuffer matrixData;

  // Inverse de la worldMatrix
  XMMATRIX worldInverse = XMMatrixInverse(nullptr, worldMatrix);
  XMMATRIX worldInverseTranspose = XMMatrixTranspose(worldInverse);

  // Calcul du WVP (worldViewProjection)
  XMMATRIX wvp = worldMatrix * viewMatrix * projMatrix;

  matrixData.modelViewProjection = XMMatrixTranspose(wvp);
  matrixData.world = XMMatrixTranspose(worldMatrix);
  matrixData.worldInverseTranspose = worldInverseTranspose;

  D3D11_MAPPED_SUBRESOURCE mappedResource;
  HRESULT result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0,
                                      &mappedResource);
  if (FAILED(result)) {
    ErrorLogger::Log("Failed to map constant buffer.");
    return false;
  }

  memcpy(mappedResource.pData, &matrixData, sizeof(matrixData));
  deviceContext->Unmap(m_matrixBuffer, 0);

  deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

  return true;
}

std::vector<std::string> BaseRendererComponent::GetActiveFeatures() const
{
  std::vector<std::string> active;
  for (auto& f : m_features) {
    active.push_back(f->GetId());
  }
  return active;
}
