#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include "dispositif.h"
#include "util.h"

namespace FrostFireEngine
{
  class DispositifD3D11 final : public CDispositif {
  public:
    DispositifD3D11(CDS_MODE cdsMode, HWND hWnd);

    ~DispositifD3D11() override
    {
      if (pSwapChain)
      {
        // S'assurer de repasser en mode fenêtré au moment de la destruction
        pSwapChain->SetFullscreenState(FALSE, NULL);
      }

      if (pImmediateContext) {
        pImmediateContext->ClearState();
      }
      DXRelacher(pRenderTargetView);
      DXRelacher(pImmediateContext);
      DXRelacher(pSwapChain);
      DXRelacher(pD3DDevice);
      DXRelacher(pDepthStencilView);
      DXRelacher(pDepthTexture);
    }

    DispositifD3D11(const DispositifD3D11 &) = delete;
    DispositifD3D11 &operator=(const DispositifD3D11 &) = delete;
    DispositifD3D11(DispositifD3D11 &&) noexcept = default;
    DispositifD3D11 &operator=(DispositifD3D11 &&) noexcept = default;

    void                        PresentSpecific() override;
    [[nodiscard]] ID3D11Device *GetD3DDevice() const
    {
      return pD3DDevice;
    }
    [[nodiscard]] ID3D11DeviceContext *GetImmediateContext() const
    {
      return pImmediateContext;
    }
    [[nodiscard]] IDXGISwapChain *GetSwapChain() const
    {
      return pSwapChain;
    }
    [[nodiscard]] ID3D11RenderTargetView *GetRenderTargetView() const
    {
      return pRenderTargetView;
    }
    [[nodiscard]] ID3D11DepthStencilView *GetDepthStencilView() const
    {
      return pDepthStencilView;
    }
    [[nodiscard]] UINT  GetWidth() const;
    [[nodiscard]] UINT  GetHeight() const;
    [[nodiscard]] float GetViewportWidth() const
    {
      return viewport.Width;
    }
    [[nodiscard]] float GetViewportHeight() const
    {
      return viewport.Height;
    }
    [[nodiscard]] const D3D11_VIEWPORT &GetViewport() const
    {
      return viewport;
    }
    void SetViewport(float width,
      float height,
      float minDepth = 0.0f,
      float maxDepth = 1.0f,
      float topLeftX = 0.0f,
      float topLeftY = 0.0f);

    void SetFullScreen(bool fullscreen, HWND hWnd);
    bool IsFullScreen() const { return isFullScreen; }

  private:
    void InitDepthBuffer();

    ID3D11Device *pD3DDevice = nullptr;
    ID3D11DeviceContext *pImmediateContext = nullptr;
    IDXGISwapChain *pSwapChain = nullptr;
    ID3D11RenderTargetView *pRenderTargetView = nullptr;
    ID3D11Texture2D *pDepthTexture = nullptr;
    ID3D11DepthStencilView *pDepthStencilView = nullptr;
    D3D11_VIEWPORT          viewport = {};

    UINT screenWidth = 0;
    UINT screenHeight = 0;
    bool isFullScreen = false;

    // Pour restaurer la taille de la fenêtre en mode fenêtré
    RECT windowedRect = { 0 };
  };
} // namespace FrostFireEngine
