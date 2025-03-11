#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>
#include "Engine/ECS/core/System.h"

namespace FrostFireEngine
{
  struct GPU_Light {
    DirectX::XMFLOAT4 colorIntensity; // xyz = color, w = intensity
    DirectX::XMFLOAT4 directionType; // xyz = direction, w = type
    DirectX::XMFLOAT4 positionRange; // xyz = position, w = range
    DirectX::XMFLOAT4 spotAnglePad; // x = spotAngle, y,z,w = pad
  };

  static const int MAX_LIGHTS = 64;

  struct LightBufferData {
    DirectX::XMFLOAT3 cameraPosition;
    float             lightCount;
    GPU_Light         lights[MAX_LIGHTS];
  };

  class LightSystem : public System {
  public:
    LightSystem(ID3D11Device* device);
    void Update(float deltaTime) override;

    void FillLightBuffer(ID3D11DeviceContext*     context,
                         const DirectX::XMFLOAT3& cameraPos,
                         const class World&       world);

    static void GetDirectionalLightMatrices(std::vector<DirectX::XMMATRIX>& matrices);

  private:
    void CreateLightBuffer();

    ID3D11Device*                        m_device;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightBuffer;
  };
}
