#pragma once
#include <DirectXMath.h>


namespace FrostFireEngine
{
  using namespace DirectX;
  struct CameraContext {
    XMMATRIX viewMatrix;
    XMMATRIX projMatrix;
    XMMATRIX viewProjMatrix;
    XMMATRIX orthoMatrix;
    XMFLOAT3 cameraPosition;
    int viewportWidth;
    int viewportHeight;
  };
}
