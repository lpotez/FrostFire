#pragma once
#include "BaseRendererComponent.h"
#include <DirectXMath.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace FrostFireEngine
{
  class UIBaseRendererComponent : public BaseRendererComponent {
  public:
    UIBaseRendererComponent() = default;
    virtual ~UIBaseRendererComponent() = default;

    // Retourne la taille de contenu finale.
    virtual float GetContentWidth() const = 0;
    virtual float GetContentHeight() const = 0;

  protected:
    bool InitializeConstantBuffers(ID3D11Device* device) override;

    void UpdateCommonConstantBuffers(ID3D11DeviceContext* deviceContext,
                                     const XMMATRIX&      worldMatrix,
                                     const XMMATRIX&      viewMatrix,
                                     const XMMATRIX&      projMatrix) const;

    void ApplyCommonUISettings(ID3D11DeviceContext* deviceContext);

    UIBaseRendererComponent(const UIBaseRendererComponent&) = delete;
    UIBaseRendererComponent& operator=(const UIBaseRendererComponent&) = delete;

    UIBaseRendererComponent(UIBaseRendererComponent&&) = default;
    UIBaseRendererComponent& operator=(UIBaseRendererComponent&&) = default;

    ComPtr<ID3D11Buffer>       m_colorBuffer;
    ComPtr<ID3D11SamplerState> m_samplerState;
    XMFLOAT4                   m_color = XMFLOAT4(1, 1, 1, 1);
  };
}
