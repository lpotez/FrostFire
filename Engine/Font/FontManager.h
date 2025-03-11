#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "Font.h"

#include "Engine/DispositifD3D11.h"
#include "Engine/Singleton.h"

namespace FrostFireEngine
{
  class FontManager : public CSingleton<FontManager> {
    friend class CSingleton;

  public:
    // Charge une fonte depuis un fichier texte des métriques et un fichier texture.
    // metricsFile: Chemin du fichier texte contenant les lignes au format :
    // [Ascii value] [Character] [LeftU] [RightU] [PixelWidth]
    // textureFile: Chemin de la texture d'atlas
    // pDispositif: Le dispositif D3D11
    // Retourne un pointeur sur la Font chargée.
    Font* LoadFont(const std::wstring& metricsFile,
                   const std::wstring& textureFile,
                   DispositifD3D11*    pDispositif);

    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

  private:
    FontManager() = default;
    ~FontManager() override = default;

    std::unordered_map<std::wstring, std::unique_ptr<Font>> m_fonts;
  };
}
