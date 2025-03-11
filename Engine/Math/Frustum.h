#pragma once
#include <DirectXMath.h>
#include <vector>

#include "AABB.h"

using namespace DirectX;

namespace FrostFireEngine
{
  class Frustum {
  public:
    void ConstructFrustum(const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix);
    bool CheckPoint(float x, float y, float z) const;
    bool CheckSphere(float x, float y, float z, float r) const;
    bool CheckCube(float x, float y, float z, float r) const;
    bool CheckRectangle(float x, float y, float z, float xSize, float ySize, float zSize) const;
    bool CheckBox(const AABB& box) const;
    void ConstructFrustumFromMatrix(const DirectX::XMMATRIX &viewProjMatrix);

  private:
    void NormalizePlane(int i);
    XMFLOAT4 m_planes[6];
  };
}
