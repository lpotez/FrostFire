#pragma once
#include "dispositifd3d11.h"
#include <string>
#include <wrl/client.h>
#include <d3d11.h>

namespace FrostFireEngine
{
  using Microsoft::WRL::ComPtr;

  class Texture {
  public:
    Texture();
    ~Texture() = default;

    // Constructeur standard (charge depuis un fichier, DDS ou WIC)
    Texture(std::wstring filename, DispositifD3D11* pDispositif, bool enableMipmaps = true);

    // Constructeur prenant directement une SRV (texture 2D)
    Texture(const std::wstring&       filename,
            DispositifD3D11*          pDispositif,
            ID3D11ShaderResourceView* srv,
            bool                      isCubeMap = false);

    [[nodiscard]] const std::wstring& GetFilename() const
    {
      return m_Filename;
    }
    [[nodiscard]] ComPtr<ID3D11ShaderResourceView> GetShaderResourceView() const
    {
      return m_Texture;
    }

    [[nodiscard]] UINT GetWidth() const;
    [[nodiscard]] UINT GetHeight() const;

  private:
    class ComInitializer {
    public:
      ComInitializer()
      {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
      }
      ~ComInitializer()
      {
        CoUninitialize();
      }
    };

    void LoadDDSTexture(const DispositifD3D11* pDispositif, bool enableMipmaps);
    void LoadWICTexture(DispositifD3D11* pDispositif, bool enableMipmaps);

    std::wstring                     m_Filename = L"";
    ComPtr<ID3D11ShaderResourceView> m_Texture;
    UINT                             m_Width = 0;
    UINT                             m_Height = 0;
    bool                             m_isCubeMap = false; // Indique si c'est une cube map
  };
}
