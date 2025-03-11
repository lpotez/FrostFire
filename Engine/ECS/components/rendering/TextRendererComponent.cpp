#include "TextRendererComponent.h"
#include "Engine/ECS/components/transform/TransformComponent.h"
#include "Engine/ECS/components/transform/RectTransformComponent.h"
#include "Engine/Shaders/ShaderVariant.h"
#include "Engine/TextureManager.h"
#include "Engine/Utils/ErrorLogger.h"

#include <vector>
#include <algorithm>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace FrostFireEngine
{
  REGISTER_INHERITANCE(TextRendererComponent, UIBaseRendererComponent)

  TextRendererComponent::TextRendererComponent(DispositifD3D11* dispositif)
    : m_dispositif(dispositif)
  {
    D3D11_INPUT_ELEMENT_DESC posElement = {
      "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
      D3D11_INPUT_PER_VERTEX_DATA, 0
    };
    D3D11_INPUT_ELEMENT_DESC uvElement = {
      "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8,
      D3D11_INPUT_PER_VERTEX_DATA, 0
    };
    m_layout.elements.push_back(posElement);
    m_layout.elements.push_back(uvElement);

    SetTechnique(std::make_unique<TextTechnique>());
    TextRendererComponent::InitializeConstantBuffers(dispositif->GetD3DDevice());
  }

  bool TextRendererComponent::InitializeConstantBuffers(ID3D11Device* device)
  {
    return UIBaseRendererComponent::InitializeConstantBuffers(device);
  }

  void TextRendererComponent::SetText(const std::wstring& text)
  {
    m_text = text;
    UpdateTextGeometry(m_dispositif->GetD3DDevice());
  }

  void TextRendererComponent::SetFont(Font* font)
  {
    m_font = font;
    UpdateTextGeometry(m_dispositif->GetD3DDevice());
  }

  void TextRendererComponent::UpdateTextGeometry(ID3D11Device* device)
  {
    if (!m_font || m_text.empty() || !device) {
      m_vertexBuffer.Reset();
      m_indexBuffer.Reset();
      m_contentWidth = 0.0f;
      m_contentHeight = 0.0f;
      return;
    }

    std::vector<UIVertex> vertices;
    std::vector<DWORD>    indices;

    float  lineHeight = m_font->GetLineHeight() * m_fontSize;
    float  penX = 0.0f;
    float  penY = 0.0f;
    float  maxLineWidth = 0.0f;
    size_t numLines = 1;

    // Calcul de la largeur max et du nombre de lignes
    {
      float currentLineWidth = 0.0f;
      for (wchar_t c : m_text) {
        if (c == L'\n') {
          maxLineWidth = std::max(maxLineWidth, currentLineWidth);
          currentLineWidth = 0.0f;
          numLines++;
          continue;
        }

        if (!m_font->HasGlyph(c)) continue;
        const Font::Glyph& glyph = m_font->GetGlyph(c);
        currentLineWidth += (glyph.xAdvance * m_fontSize);
      }
      maxLineWidth = std::max(maxLineWidth, currentLineWidth);
    }

    m_contentWidth = maxLineWidth;
    m_contentHeight = (numLines * lineHeight);

    penX = 0.0f;
    penY = 0.0f;
    size_t vertexOffset = 0;

    // Génération des vertices en coordonnées "brutes" (en pixels)
    for (size_t i = 0; i < m_text.length(); ++i) {
      wchar_t c = m_text[i];

      if (c == L'\n') {
        penX = 0.0f;
        penY += lineHeight;
        continue;
      }

      if (!m_font->HasGlyph(c)) continue;

      const Font::Glyph& glyph = m_font->GetGlyph(c);

      float gWidth = glyph.width * m_fontSize;
      float gHeight = glyph.height * m_fontSize;
      float gXAdvance = glyph.xAdvance * m_fontSize;

      float x0 = penX + glyph.xOffset * m_fontSize;
      float y0 = penY + glyph.yOffset * m_fontSize;
      float x1 = x0 + gWidth;
      float y1 = y0 + gHeight;

      UIVertex v[4];
      v[0].pos = XMFLOAT2(x0, y0);
      v[0].uv = XMFLOAT2(glyph.u0, glyph.v0);

      v[1].pos = XMFLOAT2(x1, y0);
      v[1].uv = XMFLOAT2(glyph.u1, glyph.v0);

      v[2].pos = XMFLOAT2(x0, y1);
      v[2].uv = XMFLOAT2(glyph.u0, glyph.v1);

      v[3].pos = XMFLOAT2(x1, y1);
      v[3].uv = XMFLOAT2(glyph.u1, glyph.v1);

      vertices.push_back(v[0]);
      vertices.push_back(v[1]);
      vertices.push_back(v[2]);
      vertices.push_back(v[3]);

      indices.push_back(static_cast<DWORD>(vertexOffset + 0));
      indices.push_back(static_cast<DWORD>(vertexOffset + 1));
      indices.push_back(static_cast<DWORD>(vertexOffset + 2));
      indices.push_back(static_cast<DWORD>(vertexOffset + 1));
      indices.push_back(static_cast<DWORD>(vertexOffset + 3));
      indices.push_back(static_cast<DWORD>(vertexOffset + 2));

      vertexOffset += 4;
      penX += gXAdvance;
    }

    if (vertices.empty()) {
      m_vertexBuffer.Reset();
      m_indexBuffer.Reset();
      return;
    }

    // Calcul du bounding box réel basé sur les vertices générés
    float minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
    for (auto& v : vertices) {
      if (v.pos.x < minX) minX = v.pos.x;
      if (v.pos.x > maxX) maxX = v.pos.x;
      if (v.pos.y < minY) minY = v.pos.y;
      if (v.pos.y > maxY) maxY = v.pos.y;
    }

    // Mise à jour de m_contentWidth et m_contentHeight si nécessaire
    float actualWidth = maxX - minX;
    float actualHeight = maxY - minY;

    // On se base sur actualWidth / actualHeight comme dimension finale du texte
    // afin d’être cohérent avec la normalisation qui suit
    m_contentWidth = actualWidth;
    m_contentHeight = actualHeight;

    // Normalisation des positions dans l’espace [0,1]
    for (auto& v : vertices) {
      v.pos.x = (v.pos.x - minX) / m_contentWidth;
      v.pos.y = (v.pos.y - minY) / m_contentHeight + 0.25f;
    }

    // Création des buffers
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = static_cast<UINT>(sizeof(UIVertex) * vertices.size());
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vInitData = {};
    vInitData.pSysMem = vertices.data();

    m_vertexBuffer.Reset();
    HRESULT hr = device->CreateBuffer(&vbd, &vInitData, &m_vertexBuffer);
    if (FAILED(hr)) {
      ErrorLogger::Log(hr, "Failed to create vertex buffer in TextRenderer::UpdateTextGeometry");
      return;
    }

    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = static_cast<UINT>(sizeof(DWORD) * indices.size());
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA iInitData = {};
    iInitData.pSysMem = indices.data();

    m_indexBuffer.Reset();
    hr = device->CreateBuffer(&ibd, &iInitData, &m_indexBuffer);
    if (FAILED(hr)) {
      ErrorLogger::Log(hr, "Failed to create index buffer in TextRenderer::UpdateTextGeometry");
    }

  }
  void TextRendererComponent::Draw(ID3D11DeviceContext* deviceContext,
                                   const XMMATRIX&      viewMatrix,
                                   const XMMATRIX&      projectionMatrix,
                                   RenderPass           currentPass)
  {
    if (!IsVisible() || !m_font || !m_vertexBuffer || !m_indexBuffer) return;

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
    if (!variant) return;

    variant->Apply(deviceContext);

    ID3D11ShaderResourceView* srv = m_font->GetAtlasSRV();
    deviceContext->PSSetShaderResources(0, 1, &srv);

    ApplyCommonUISettings(deviceContext);

    UINT stride = sizeof(UIVertex);
    UINT offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    size_t glyphCount = 0;
    for (wchar_t c : m_text) {
      if (c != L'\n' && m_font->HasGlyph(c)) {
        glyphCount++;
      }
    }

    deviceContext->DrawIndexed(static_cast<UINT>(glyphCount * 6), 0, 0);
  }


  const VertexLayoutDesc& TextRendererComponent::GetVertexLayout() const
  {
    return m_layout;
  }

  ShaderTechnique* TextRendererComponent::GetTechnique() const
  {
    return m_technique;
  }
}
