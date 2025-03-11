#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <unordered_map>

namespace FrostFireEngine
{
  using Microsoft::WRL::ComPtr;

  class Font {
  public:
    struct Glyph {
      float u0,      v0, u1, v1;
      float xOffset, yOffset;
      float xAdvance;
      float width, height;
    };

    ID3D11ShaderResourceView* GetAtlasSRV() const
    {
      return m_fontAtlasSRV.Get();
    }
    const Glyph& GetGlyph(wchar_t c) const
    {
      return m_glyphs.at(c);
    }
    bool HasGlyph(wchar_t c) const
    {
      return m_glyphs.find(c) != m_glyphs.end();
    }

    float GetLineHeight() const
    {
      return m_lineHeight;
    }

    void AddGlyph(wchar_t c, const Glyph& g)
    {
      m_glyphs[c] = g;
    }
    void SetAtlasSRV(ComPtr<ID3D11ShaderResourceView> srv)
    {
      m_fontAtlasSRV = srv;
    }
    void SetLineHeight(float lh)
    {
      m_lineHeight = lh;
    }

  private:
    std::unordered_map<wchar_t, Glyph> m_glyphs;
    ComPtr<ID3D11ShaderResourceView>   m_fontAtlasSRV;
    float                              m_lineHeight = 0.0f;
  };
}
