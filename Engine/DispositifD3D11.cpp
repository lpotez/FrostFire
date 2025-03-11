#include "StdAfx.h"
#include "resource.h"
#include "DispositifD3D11.h"
#include "Util.h"
namespace FrostFireEngine
{
  DispositifD3D11::DispositifD3D11(const CDS_MODE cdsMode, const HWND hWnd)
  {
    int width = 0;
    int height = 0;
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL featureLevels[] =
    {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
    };
    constexpr UINT numFeatureLevels = ARRAYSIZE(featureLevels);
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    switch (cdsMode)
    {
    case CDS_FENETRE:
    {
      isFullScreen = false;
      GetWindowRect(hWnd, &windowedRect);
      RECT rcClient;
      GetClientRect(hWnd, &rcClient);
      width = rcClient.right - rcClient.left;
      height = rcClient.bottom - rcClient.top;
      sd.BufferCount = 1;
      sd.BufferDesc.Width = width;
      sd.BufferDesc.Height = height;
      sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      sd.BufferDesc.RefreshRate.Numerator = 60;
      sd.BufferDesc.RefreshRate.Denominator = 1;
      sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      sd.OutputWindow = hWnd;
      sd.SampleDesc.Count = 1;
      sd.SampleDesc.Quality = 0;
      sd.Windowed = TRUE;
      break;
    }
    case CDS_PLEIN_ECRAN:
    {
      isFullScreen = true;
      width = GetSystemMetrics(SM_CXSCREEN);
      height = GetSystemMetrics(SM_CYSCREEN);
      GetWindowRect(hWnd, &windowedRect);
      sd.BufferCount = 1;
      sd.BufferDesc.Width = width;
      sd.BufferDesc.Height = height;
      sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      sd.BufferDesc.RefreshRate.Numerator = 60;
      sd.BufferDesc.RefreshRate.Denominator = 1;
      sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      sd.OutputWindow = hWnd;
      sd.SampleDesc.Count = 1;
      sd.SampleDesc.Quality = 0;
      sd.Windowed = FALSE;
      break;
    }
    }
    DXEssayer(D3D11CreateDeviceAndSwapChain(nullptr,
      D3D_DRIVER_TYPE_HARDWARE,
      nullptr,
      createDeviceFlags,
      featureLevels, numFeatureLevels,
      D3D11_SDK_VERSION,
      &sd,
      &pSwapChain,
      &pD3DDevice,
      nullptr,
      &pImmediateContext),
      DXE_ERREURCREATIONDEVICE);
    ID3D11Texture2D *pBackBuffer = nullptr;
    DXEssayer(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
      reinterpret_cast<void **>(&pBackBuffer)),
      DXE_ERREUROBTENTIONBUFFER);
    DXEssayer(pD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView),
      DXE_ERREURCREATIONRENDERTARGET);
    DXRelacher(pBackBuffer);
    screenWidth = width;
    screenHeight = height;
    InitDepthBuffer();
    pImmediateContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);
    viewport.Width = static_cast<FLOAT>(width);
    viewport.Height = static_cast<FLOAT>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    pImmediateContext->RSSetViewports(1, &viewport);
    D3D11_RASTERIZER_DESC rsDesc;
    ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.FrontCounterClockwise = false;
    ID3D11RasterizerState *rs;
    DXEssayer(pD3DDevice->CreateRasterizerState(&rsDesc, &rs),
      DXE_ERREURCREATIONRASTERIZER);
    pImmediateContext->RSSetState(rs);
    DXRelacher(rs);
  }
  void DispositifD3D11::SetFullScreen(bool fullscreen, HWND hWnd)
  {
    if (fullscreen == isFullScreen)
      return;
    if (fullscreen)
    {
      pSwapChain->SetFullscreenState(TRUE, NULL);
      isFullScreen = true;
    }
    else
    {
      pSwapChain->SetFullscreenState(FALSE, NULL);
      MoveWindow(hWnd, windowedRect.left, windowedRect.top,
        windowedRect.right - windowedRect.left,
        windowedRect.bottom - windowedRect.top,
        TRUE);
      isFullScreen = false;
    }
  }
  void DispositifD3D11::PresentSpecific()
  {
    pSwapChain->Present(0, 0);
  }
  UINT DispositifD3D11::GetWidth() const
  {
    DXGI_SWAP_CHAIN_DESC desc;
    pSwapChain->GetDesc(&desc);
    return desc.BufferDesc.Width;
  }
  UINT DispositifD3D11::GetHeight() const
  {
    DXGI_SWAP_CHAIN_DESC desc;
    pSwapChain->GetDesc(&desc);
    return desc.BufferDesc.Height;
  }
  void DispositifD3D11::SetViewport(float width, float height, float minDepth, float maxDepth,
    float topLeftX, float topLeftY)
  {
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = minDepth;
    viewport.MaxDepth = maxDepth;
    viewport.TopLeftX = topLeftX;
    viewport.TopLeftY = topLeftY;
    pImmediateContext->RSSetViewports(1, &viewport);
  }
  void DispositifD3D11::InitDepthBuffer()
  {
    D3D11_TEXTURE2D_DESC depthTextureDesc;
    ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
    depthTextureDesc.Width = screenWidth;
    depthTextureDesc.Height = screenHeight;
    depthTextureDesc.MipLevels = 1;
    depthTextureDesc.ArraySize = 1;
    depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthTextureDesc.SampleDesc.Count = 1;
    depthTextureDesc.SampleDesc.Quality = 0;
    depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthTextureDesc.CPUAccessFlags = 0;
    depthTextureDesc.MiscFlags = 0;
    DXEssayer(pD3DDevice->CreateTexture2D(&depthTextureDesc, nullptr, &pDepthTexture),
      DXE_ERREURCREATIONTEXTURE);
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSView;
    ZeroMemory(&descDSView, sizeof(descDSView));
    descDSView.Format = depthTextureDesc.Format;
    descDSView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSView.Texture2D.MipSlice = 0;
    DXEssayer(pD3DDevice->CreateDepthStencilView(pDepthTexture,
      &descDSView,
      &pDepthStencilView),
      DXE_ERREURCREATIONDEPTHSTENCILTARGET);
  }
}
