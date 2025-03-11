#include "D3DResources.h"


namespace FrostFireEngine
{
  void D3DResources::setContext(const ID3D11DeviceContext* context)
  {
    deviceContext = context;
  }
  void D3DResources::setDevice(const ID3D11Device* _device)
  {
    device = _device;
  }
  const ID3D11Device* D3DResources::GetDevice() const
  {
    return device;
    //return nullptr;
  }
  const ID3D11DeviceContext* D3DResources::GetContext()
  {
    return deviceContext;
  }
}
