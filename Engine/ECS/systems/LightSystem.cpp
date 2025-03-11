#include "LightSystem.h"
#include "Engine/ECS/components/LightComponent.h"
#include "Engine/ECS/core/World.h"
#include "Engine/ECS/components/transform/TransformComponent.h"

using namespace DirectX;
using namespace FrostFireEngine;

static const size_t MAX_DIR_LIGHTS = 4;

LightSystem::LightSystem(ID3D11Device* device) : m_device(device)
{
  CreateLightBuffer();
}

void LightSystem::Update(float /*deltaTime*/)
{
  // Mise à jour si nécessaire
}

void LightSystem::FillLightBuffer(ID3D11DeviceContext* context,
                                  const XMFLOAT3&      cameraPos,
                                  const World&         world)
{
  // Sépare les lumières directionnelles des autres
  std::vector<LightComponent*> directionalLights;
  std::vector<LightComponent*> otherLights;

  world.ForEachComponent<LightComponent>([&](LightComponent* lc)
  {
    if (lc->GetType() == LightType::Directional) {
      directionalLights.push_back(lc);
    }
    else {
      otherLights.push_back(lc);
    }
  });

  // Concatène : directionnelles d'abord, puis autres
  std::vector<LightComponent*> allLights;
  allLights.reserve(directionalLights.size() + otherLights.size());
  allLights.insert(allLights.end(), directionalLights.begin(), directionalLights.end());
  allLights.insert(allLights.end(), otherLights.begin(), otherLights.end());

  LightBufferData data = {};
  data.cameraPosition = cameraPos;
  data.lightCount = static_cast<float>(std::min<size_t>(allLights.size(), MAX_LIGHTS));

  for (size_t i = 0; i < std::min<size_t>(allLights.size(), MAX_LIGHTS); ++i) {
    const LightComponent* lc = allLights[i];

    // Récupère les propriétés de la lumière
    XMFLOAT3 color = lc->GetColor();
    float    intensity = lc->GetIntensity();

    XMFLOAT3 direction = lc->GetDirection();
    float    type;
    if (lc->GetType() == LightType::Directional) {
      type = 0.0f;
    }
    else if (lc->GetType() == LightType::Point) {
      type = 1.0f;
    }
    else {
      type = 2.0f;
    }

    XMFLOAT3 position(0.0f, 0.0f, 0.0f);
    if (const auto entity = world.GetEntity(lc->GetOwner())) {
      if (const auto transform = entity->GetComponent<TransformComponent>()) {
        XMStoreFloat3(&position, transform->GetWorldPosition());
      }
    }

    float range = lc->GetRange();
    float spotAngle = lc->GetSpotAngle();

    // Remplir la structure GPU_Light
    data.lights[i].colorIntensity = XMFLOAT4(color.x, color.y, color.z, intensity);
    data.lights[i].directionType = XMFLOAT4(direction.x, direction.y, direction.z, type);
    data.lights[i].positionRange = XMFLOAT4(position.x, position.y, position.z, range);
    data.lights[i].spotAnglePad = XMFLOAT4(spotAngle, 0.0f, 0.0f, 0.0f);
  }

  D3D11_MAPPED_SUBRESOURCE mappedRes;
  HRESULT hr = context->Map(m_lightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);
  if (SUCCEEDED(hr)) {
    memcpy(mappedRes.pData, &data, sizeof(LightBufferData));
    context->Unmap(m_lightBuffer.Get(), 0);
  }

  context->PSSetConstantBuffers(3, 1, m_lightBuffer.GetAddressOf());
}

void LightSystem::GetDirectionalLightMatrices(std::vector<XMMATRIX>& matrices)
{
  matrices.clear();
  World::GetInstance().ForEachComponent<LightComponent>([&](LightComponent* lc)
  {
    if (lc->GetType() == LightType::Directional && matrices.size() < MAX_DIR_LIGHTS) {
      XMFLOAT3 dirF = lc->GetDirection();
      XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&dirF));

      XMFLOAT3 lightPosF(50.f, 50.f, 50.f);
      if (auto entity = World::GetInstance().GetEntity(lc->GetOwner())) {
        if (auto transform = entity->GetComponent<TransformComponent>()) {
          XMStoreFloat3(&lightPosF, transform->GetWorldPosition());
        }
      }

      XMVECTOR lightPos = XMLoadFloat3(&lightPosF);
      XMVECTOR up = XMVectorSet(0, 1, 0, 0);
      XMMATRIX lightView = XMMatrixLookAtLH(lightPos, XMVectorAdd(lightPos, lightDir), up);
      XMMATRIX lightProj = XMMatrixOrthographicLH(300.0f, 300.0f, 0.1f, 700.0f);
      matrices.push_back(XMMatrixMultiply(lightView, lightProj));
    }
  });
}

void LightSystem::CreateLightBuffer()
{
  D3D11_BUFFER_DESC bd = {};
  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.ByteWidth = sizeof(LightBufferData);
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bd.MiscFlags = 0;

  HRESULT hr = m_device->CreateBuffer(&bd, nullptr, &m_lightBuffer);
  if (FAILED(hr)) {
  }
}
