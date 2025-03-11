#include "FallbackTextures.h"
#include "Engine/Utils/ErrorLogger.h"
#include "Engine/TextureManager.h"
#include "Engine/Texture.h"
#include <wrl/client.h>
#include <string>

namespace FrostFireEngine
{
  // Crée une texture 2D blanche 1x1
  ID3D11ShaderResourceView* CreateWhiteTexture(ID3D11Device* device)
  {
    constexpr UINT whitePixel[1] = {0xFFFFFFFF};

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = 1;
    texDesc.Height = 1;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = whitePixel;
    initData.SysMemPitch = sizeof(UINT);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
    HRESULT                                 hr = device->CreateTexture2D(&texDesc, &initData, &tex);
    if (FAILED(hr)) {
      ErrorLogger::Log("Failed to create white texture 2D");
      return nullptr;
    }

    ID3D11ShaderResourceView* srv = nullptr;
    hr = device->CreateShaderResourceView(tex.Get(), nullptr, &srv);
    if (FAILED(hr)) {
      ErrorLogger::Log("Failed to create SRV for white texture");
      return nullptr;
    }

    return srv;
  }

  void InitializeWhiteFallbackTexture(DispositifD3D11* device)
  {
    auto& texManager = TextureManager::GetInstance();
    if (!texManager.GetTexture(L"__white_fallback__")) {
      if (ID3D11ShaderResourceView* whiteSRV = CreateWhiteTexture(device->GetD3DDevice())) {
        auto whiteTexture = std::make_unique<Texture>(L"__white_fallback__", device, whiteSRV,
                                                      false);
        texManager.AddTexture(std::move(whiteTexture));
      }
      else {
        ErrorLogger::Log("Failed to create white fallback texture.");
      }
    }
  }

  // Crée une cube map 1x1 blanche (6 faces)
  ID3D11ShaderResourceView* CreateWhiteCubeMap(ID3D11Device* device)
  {
    const UINT whitePixel[1] = {0xFFFFFFFF};

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = 1;
    texDesc.Height = 1;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 6; // 6 faces
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    D3D11_SUBRESOURCE_DATA initData[6] = {};
    for (int i = 0; i < 6; i++) {
      initData[i].pSysMem = whitePixel;
      initData[i].SysMemPitch = sizeof(UINT);
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
    HRESULT                                 hr = device->CreateTexture2D(&texDesc, initData, &tex);
    if (FAILED(hr)) {
      ErrorLogger::Log("Failed to create white cube map texture 2D");
      return nullptr;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels = 1;
    srvDesc.TextureCube.MostDetailedMip = 0;

    ID3D11ShaderResourceView* srv = nullptr;
    hr = device->CreateShaderResourceView(tex.Get(), &srvDesc, &srv);
    if (FAILED(hr)) {
      ErrorLogger::Log("Failed to create SRV for white cube map texture");
      return nullptr;
    }

    return srv;
  }

  void InitializeWhiteCubeMapFallbackTexture(DispositifD3D11* device)
  {
    auto& texManager = TextureManager::GetInstance();
    if (!texManager.GetTexture(L"__white_cubemap_fallback__")) {
      if (ID3D11ShaderResourceView* whiteSRV = CreateWhiteCubeMap(device->GetD3DDevice())) {
        auto whiteCubeTexture = std::make_unique<Texture>(L"__white_cubemap_fallback__", device,
                                                          whiteSRV, true);
        texManager.AddTexture(std::move(whiteCubeTexture));
      }
      else {
        ErrorLogger::Log("Failed to create white cube map fallback texture.");
      }
    }
  }

  // Crée une texture pour la normal map neutre (0.5,0.5,1.0)
  ID3D11ShaderResourceView* CreateNeutralNormalTexture(ID3D11Device* device)
  {
    // Valeur de normale neutre en 8 bits : (128, 128, 255, 255)
    const unsigned char normalPixel[4] = {128, 128, 255, 255};

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = 1;
    texDesc.Height = 1;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = normalPixel;
    initData.SysMemPitch = 4 * sizeof(unsigned char);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
    HRESULT                                 hr = device->CreateTexture2D(&texDesc, &initData, &tex);
    if (FAILED(hr)) {
      ErrorLogger::Log("Failed to create neutral normal texture 2D");
      return nullptr;
    }

    ID3D11ShaderResourceView* srv = nullptr;
    hr = device->CreateShaderResourceView(tex.Get(), nullptr, &srv);
    if (FAILED(hr)) {
      ErrorLogger::Log("Failed to create SRV for neutral normal texture");
      return nullptr;
    }

    return srv;
  }

  void InitializeNeutralNormalFallbackTexture(DispositifD3D11* device)
  {
    auto& texManager = TextureManager::GetInstance();
    if (!texManager.GetTexture(L"__neutralnormal_fallback__")) {
      if (ID3D11ShaderResourceView* normalSRV =
        CreateNeutralNormalTexture(device->GetD3DDevice())) {
        auto normalTexture = std::make_unique<Texture>(L"__neutralnormal_fallback__", device,
                                                       normalSRV, false);
        texManager.AddTexture(std::move(normalTexture));
      }
      else {
        ErrorLogger::Log("Failed to create neutral normal fallback texture.");
      }
    }
  }
}
