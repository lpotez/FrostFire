#pragma once
#include "UIBaseRendererComponent.h"
#include "Engine/Shaders/Features/RenderPass.h"
#include <DirectXMath.h>
#include <wrl/client.h>

namespace FrostFireEngine
{
  class Texture;
  class TransformComponent;
  class RectTransformComponent;
  class DispositifD3D11;

  class UIRendererComponent : public UIBaseRendererComponent {
  public:
    UIRendererComponent(const DispositifD3D11* dispositif);
    ~UIRendererComponent() override;

    void Draw(ID3D11DeviceContext* deviceContext,
              const XMMATRIX&      viewMatrix,
              const XMMATRIX&      projectionMatrix,
              RenderPass           currentPass) override;

    const VertexLayoutDesc& GetVertexLayout() const override;
    ShaderTechnique*        GetTechnique() const override;

    void SetTexture(Texture* texture);
    void SetColor(const XMFLOAT4& color)
    {
      m_color = color;
    }

    float GetContentWidth() const override
    {
      return m_contentWidth;
    }
    float GetContentHeight() const override
    {
      return m_contentHeight;
    }

    UIRendererComponent(UIRendererComponent&& other) noexcept = default;
    UIRendererComponent& operator=(UIRendererComponent&& other) noexcept = default;

    UIRendererComponent(const UIRendererComponent&) = delete;
    UIRendererComponent& operator=(const UIRendererComponent&) = delete;

  protected:
    bool InitializeConstantBuffers(ID3D11Device* device) override;

  private:
    struct UIVertex {
      XMFLOAT2 pos;
      XMFLOAT2 uv;
    };

    static VertexLayoutDesc CreateUILayout();
    void                    CreateQuadGeometry(ID3D11Device* device);

    Texture* m_texture = nullptr;
    float    m_contentWidth = 0.0f;
    float    m_contentHeight = 0.0f;

    const DispositifD3D11* m_dispositif = nullptr;
    VertexLayoutDesc       m_layout;
    ComPtr<ID3D11Buffer>   m_vertexBuffer;
    ComPtr<ID3D11Buffer>   m_indexBuffer;
  };
}
