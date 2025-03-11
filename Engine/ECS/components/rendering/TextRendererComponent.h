#pragma once
#include "UIBaseRendererComponent.h"
#include <string>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "Engine/DispositifD3D11.h"
#include "Engine/Font/Font.h"
#include "Engine/Shaders/techniques/TextTechnique.h"

namespace FrostFireEngine
{
  struct UIVertex {
    XMFLOAT2 pos;
    XMFLOAT2 uv;
  };

  class TextRendererComponent : public UIBaseRendererComponent {
  public:
    TextRendererComponent(DispositifD3D11* dispositif);
    ~TextRendererComponent() override = default;

    TextRendererComponent(TextRendererComponent&& other) noexcept = default;
    TextRendererComponent& operator=(TextRendererComponent&& other) noexcept = default;

    TextRendererComponent(const TextRendererComponent&) = delete;
    TextRendererComponent& operator=(const TextRendererComponent&) = delete;

    void                SetText(const std::wstring& text);
    const std::wstring& GetText() const
    {
      return m_text;
    }

    void  SetFont(Font* font);
    Font* GetFont() const
    {
      return m_font;
    }

    void SetColor(const XMFLOAT4& color)
    {
      m_color = color;
    }
    const XMFLOAT4& GetColor() const
    {
      return m_color;
    }

    XMFLOAT2 SetFontSize(float size)
    {
      m_fontSize = size;
      UpdateTextGeometry(m_dispositif->GetD3DDevice());
      return { m_contentWidth, m_contentHeight };
    }
    float GetFontSize() const
    {
      return m_fontSize;
    }

    void Draw(ID3D11DeviceContext* deviceContext,
              const XMMATRIX&      viewMatrix,
              const XMMATRIX&      projectionMatrix,
              RenderPass           currentPass) override;

    const VertexLayoutDesc& GetVertexLayout() const override;
    ShaderTechnique*        GetTechnique() const override;

    float GetContentWidth() const override
    {
      return m_contentWidth;
    }
    float GetContentHeight() const override
    {
      return m_contentHeight;
    }

  protected:
    bool InitializeConstantBuffers(ID3D11Device* device) override;

  private:
    void UpdateTextGeometry(ID3D11Device* device);

    Font*        m_font = nullptr;
    std::wstring m_text;
    float        m_contentWidth = 0.0f;
    float        m_contentHeight = 0.0f;
    float        m_fontSize = 6.0f;

    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_indexBuffer;

    DispositifD3D11* m_dispositif = nullptr;
    VertexLayoutDesc m_layout;
  };
}
