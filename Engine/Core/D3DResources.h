#pragma once
#include <d3d11.h>
#include <wrl/client.h>


namespace FrostFireEngine
{
  using Microsoft::WRL::ComPtr;

  class D3DResources {
    ComPtr<ID3D11BlendState>   blendStates;
    ComPtr<ID3D11SamplerState> samplerStates;
    ComPtr<ID3D11Buffer>       sharedBuffers;
    const ID3D11Device*        device = nullptr;
    const ID3D11DeviceContext* deviceContext = nullptr;

  public:
    void                       setContext(const ID3D11DeviceContext* context);
    void                       setDevice(const ID3D11Device* _device);
    const ID3D11Device*        GetDevice() const;
    const ID3D11DeviceContext* GetContext();
  };
}
