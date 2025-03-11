#include "FontManager.h"
#include "Engine/Utils/ErrorLogger.h"
#include <fstream>
#include <sstream>

#include "Engine/Texture.h"

namespace FrostFireEngine
{
  Font* FontManager::LoadFont(const std::wstring& metricsFile,
                              const std::wstring& textureFile,
                              DispositifD3D11*    pDispositif)
  {
    std::wstring key = metricsFile + L"#" + textureFile;
    if (m_fonts.contains(key)) return m_fonts[key].get();

    Texture atlasTexture(textureFile, pDispositif);
    UINT    texWidth = atlasTexture.GetWidth();
    UINT    texHeight = atlasTexture.GetHeight();

    if (texWidth == 0 || texHeight == 0) {
      ErrorLogger::Log("FontManager: Texture atlas is empty or failed to load.");
      return nullptr;
    }

    std::ifstream file(metricsFile);
    if (!file.is_open()) {
      ErrorLogger::Log("FontManager: Could not open metrics file.");
      return nullptr;
    }

    constexpr float DEFAULT_LINE_HEIGHT = 1.0f;
    const float TEXTURE_TO_NORMALIZED = DEFAULT_LINE_HEIGHT / static_cast<float>(texHeight);

    auto font = std::make_unique<Font>();
    font->SetAtlasSRV(atlasTexture.GetShaderResourceView());
    font->SetLineHeight(DEFAULT_LINE_HEIGHT);

    std::string line;
    while (std::getline(file, line)) {
      if (line.empty()) continue;
      std::istringstream iss(line);
      int                asciiVal;
      char               character;
      float              leftU, rightU;
      float              pixelWidth;

      if (!(iss >> asciiVal >> character >> leftU >> rightU >> pixelWidth)) {
        continue;
      }

      Font::Glyph g;
      g.u0 = leftU;
      g.v0 = 0.0f;
      g.u1 = rightU;
      g.v1 = 1.0f;

      // Les dimensions sont maintenant normalisées par rapport à la line height
      g.xOffset = 0.0f;
      g.yOffset = 0.0f;
      g.width = pixelWidth * TEXTURE_TO_NORMALIZED;
      g.height = DEFAULT_LINE_HEIGHT;
      g.xAdvance = g.width; // ou ajuster si besoin d'un espacement différent

      font->AddGlyph(static_cast<wchar_t>(asciiVal), g);
    }

    file.close();

    m_fonts[key] = std::move(font);
    return m_fonts[key].get();
  }
}
