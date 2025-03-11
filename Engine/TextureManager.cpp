#include "TextureManager.h"

#include <cassert>

#include "DispositifD3D11.h"
#include "Texture.h"

namespace FrostFireEngine
{
  Texture* TextureManager::GetNewTexture(const std::wstring& filename, DispositifD3D11* pDispositif)
  {
    // Vérifie si la texture est déjà dans la liste
    Texture* pTexture = GetTexture(filename);
    // Si non, crée-la
    if (!pTexture) {
      auto texture = std::make_unique<Texture>(filename, pDispositif);
      pTexture = texture.get();
      // Ajoute la texture à la liste
      ListeTextures.push_back(std::move(texture));
    }
    assert(pTexture);
    return pTexture;
  }

  Texture* TextureManager::GetTexture(const std::wstring& filename) const
  {
    for (const auto& texture : ListeTextures) {
      if (texture->GetFilename() == filename) {
        return texture.get();
      }
    }
    return nullptr;
  }

  void TextureManager::AddTexture(std::unique_ptr<Texture> texture)
  {
    ListeTextures.push_back(std::move(texture));
  }

  TextureManager::~TextureManager()
  {
    ListeTextures.clear();
  }
}
