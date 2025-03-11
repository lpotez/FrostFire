#include "stdafx.h"
#include "Texture.h"

#include <codecvt>
#include <vector>
#include "util.h"
#include <wincodec.h>
#include "DDSTextureLoader11.h"
#include "Utils/ErrorLogger.h"
#include "Utils/WStringUtils.h"
using namespace DirectX;


namespace FrostFireEngine
{
  Texture::Texture()
    : m_Texture(nullptr)
  {
  }

  Texture::Texture(std::wstring filename, DispositifD3D11* pDispositif, bool enableMipmaps)
    : m_Filename(std::move(filename))
      , m_Texture(nullptr)
  {
    if (!pDispositif || !pDispositif->GetD3DDevice() || !pDispositif->GetImmediateContext()) return;

    std::wstring extension = m_Filename.substr(m_Filename.find_last_of(L".") + 1);

    if (_wcsicmp(extension.c_str(), L"dds") == 0) {
      LoadDDSTexture(pDispositif, enableMipmaps);
    }
    else {
      ComInitializer comInit;
      LoadWICTexture(pDispositif, enableMipmaps);
    }
  }

  Texture::Texture(const std::basic_string<wchar_t>& filename,
                   DispositifD3D11*,
                   ID3D11ShaderResourceView* srv,
                   bool                      isCubeMap)
    : m_Filename(filename)
      , m_Texture(srv)
      , m_Width(1)
      , m_Height(1)
      , m_isCubeMap(isCubeMap)
  {
    // Ici on définit arbitrairement width/height à 1 si c'est une texture créée manuellement.
  }

  UINT Texture::GetWidth() const
  {
    if (!m_Texture) return 0;

    ComPtr<ID3D11Resource> resource;
    m_Texture->GetResource(&resource);

    ComPtr<ID3D11Texture2D> texture2D;
    resource.As(&texture2D);

    D3D11_TEXTURE2D_DESC desc;
    texture2D->GetDesc(&desc);

    return desc.Width;
  }

  UINT Texture::GetHeight() const
  {
    if (!m_Texture) return 0;

    ComPtr<ID3D11Resource> resource;
    m_Texture->GetResource(&resource);

    ComPtr<ID3D11Texture2D> texture2D;
    resource.As(&texture2D);

    D3D11_TEXTURE2D_DESC desc;
    texture2D->GetDesc(&desc);

    return desc.Height;
  }

  void Texture::LoadDDSTexture(const DispositifD3D11* pDispositif, bool enableMipmaps)
  {
    ID3D11Device*        pDevice = pDispositif->GetD3DDevice();
    ID3D11DeviceContext* pContext = pDispositif->GetImmediateContext();

    ComPtr<ID3D11Resource> resource;
    HRESULT                hr = CreateDDSTextureFromFile(
      pDevice,
      pContext,
      m_Filename.c_str(),
      &resource,
      m_Texture.GetAddressOf()
    );

    if (FAILED(hr)) {
      std::string message = ConvertWStringToString(
        L"Echec du chargement de la texture DDS: " + m_Filename);
      ErrorLogger::Log(message);
      return;
    }

    // Récupérer la description de la texture pour connaître sa taille et vérifier le cube map
    ComPtr<ID3D11Texture2D> texture2D;
    resource.As(&texture2D);
    D3D11_TEXTURE2D_DESC desc;
    texture2D->GetDesc(&desc);

    m_Width = desc.Width;
    m_Height = desc.Height;

    if (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) {
      m_isCubeMap = true;
    }
    else {
      m_isCubeMap = false;
    }

    if (enableMipmaps) {
      pContext->GenerateMips(m_Texture.Get());
    }
  }

  void Texture::LoadWICTexture(DispositifD3D11* pDispositif, bool enableMipmaps)
  {
    ID3D11Device*        pDevice = pDispositif->GetD3DDevice();
    ID3D11DeviceContext* pContext = pDispositif->GetImmediateContext();

    ComPtr<IWICImagingFactory> pFactory;
    HRESULT                    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                                                     CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
    if (FAILED(hr)) return;

    ComPtr<IWICBitmapDecoder> pDecoder;
    hr = pFactory->CreateDecoderFromFilename(m_Filename.c_str(),
                                             nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad,
                                             &pDecoder);
    if (FAILED(hr)) return;

    ComPtr<IWICBitmapFrameDecode> pFrame;
    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr)) return;

    ComPtr<IWICFormatConverter> pConverter;
    hr = pFactory->CreateFormatConverter(&pConverter);
    if (FAILED(hr)) return;

    hr = pConverter->Initialize(pFrame.Get(), GUID_WICPixelFormat32bppRGBA,
                                WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) return;

    hr = pConverter->GetSize(&m_Width, &m_Height);
    if (FAILED(hr)) return;

    std::vector<BYTE> pixels(m_Width * m_Height * 4);
    hr = pConverter->CopyPixels(nullptr, m_Width * 4,
                                static_cast<UINT>(pixels.size()), pixels.data());
    if (FAILED(hr)) return;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = m_Width;
    desc.Height = m_Height;
    desc.MipLevels = enableMipmaps ? 0 : 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | (enableMipmaps ? D3D11_BIND_RENDER_TARGET : 0);
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = enableMipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels.data();
    initData.SysMemPitch = m_Width * 4;

    ComPtr<ID3D11Texture2D> pTexture2D;
    hr = pDevice->CreateTexture2D(&desc, enableMipmaps ? nullptr : &initData, &pTexture2D);
    if (FAILED(hr)) return;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = enableMipmaps ? -1 : desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;

    hr = pDevice->CreateShaderResourceView(pTexture2D.Get(), &srvDesc, m_Texture.GetAddressOf());
    if (FAILED(hr)) return;

    if (enableMipmaps) {
      pContext->UpdateSubresource(pTexture2D.Get(), 0, nullptr, pixels.data(), m_Width * 4, 0);
      pContext->GenerateMips(m_Texture.Get());
    }
  }
}
