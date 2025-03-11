#pragma once
#include <memory>
#include <vector>

#include "Singleton.h"
#include "Texture.h"

namespace FrostFireEngine
{
  class DispositifD3D11;

  class TextureManager : public CSingleton<TextureManager>
  {
    friend class CSingleton<TextureManager>;

  public:
    Texture* GetNewTexture(const std::wstring& filename, DispositifD3D11* pDispositif);
    Texture* GetTexture(const std::wstring& filename) const;
    void AddTexture(std::unique_ptr<Texture> texture);

    ~TextureManager() override;
    TextureManager() = default;

  private:
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    std::vector<std::unique_ptr<Texture>> ListeTextures;
  };
}
