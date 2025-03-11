#include "Frustum.h"
#include <cmath>

namespace FrostFireEngine
{
  void Frustum::ConstructFrustum(const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix)
  {
    XMMATRIX viewProj = XMMatrixMultiply(viewMatrix, projectionMatrix);

    // Left plane
    m_planes[0].x = viewProj.r[0].m128_f32[3] + viewProj.r[0].m128_f32[0];
    m_planes[0].y = viewProj.r[1].m128_f32[3] + viewProj.r[1].m128_f32[0];
    m_planes[0].z = viewProj.r[2].m128_f32[3] + viewProj.r[2].m128_f32[0];
    m_planes[0].w = viewProj.r[3].m128_f32[3] + viewProj.r[3].m128_f32[0];
    NormalizePlane(0);

    // Right plane
    m_planes[1].x = viewProj.r[0].m128_f32[3] - viewProj.r[0].m128_f32[0];
    m_planes[1].y = viewProj.r[1].m128_f32[3] - viewProj.r[1].m128_f32[0];
    m_planes[1].z = viewProj.r[2].m128_f32[3] - viewProj.r[2].m128_f32[0];
    m_planes[1].w = viewProj.r[3].m128_f32[3] - viewProj.r[3].m128_f32[0];
    NormalizePlane(1);

    // Bottom plane
    m_planes[2].x = viewProj.r[0].m128_f32[3] + viewProj.r[0].m128_f32[1];
    m_planes[2].y = viewProj.r[1].m128_f32[3] + viewProj.r[1].m128_f32[1];
    m_planes[2].z = viewProj.r[2].m128_f32[3] + viewProj.r[2].m128_f32[1];
    m_planes[2].w = viewProj.r[3].m128_f32[3] + viewProj.r[3].m128_f32[1];
    NormalizePlane(2);

    // Top plane
    m_planes[3].x = viewProj.r[0].m128_f32[3] - viewProj.r[0].m128_f32[1];
    m_planes[3].y = viewProj.r[1].m128_f32[3] - viewProj.r[1].m128_f32[1];
    m_planes[3].z = viewProj.r[2].m128_f32[3] - viewProj.r[2].m128_f32[1];
    m_planes[3].w = viewProj.r[3].m128_f32[3] - viewProj.r[3].m128_f32[1];
    NormalizePlane(3);

    // Near plane
    m_planes[4].x = viewProj.r[0].m128_f32[2];
    m_planes[4].y = viewProj.r[1].m128_f32[2];
    m_planes[4].z = viewProj.r[2].m128_f32[2];
    m_planes[4].w = viewProj.r[3].m128_f32[2];
    NormalizePlane(4);

    // Far plane
    m_planes[5].x = viewProj.r[0].m128_f32[3] - viewProj.r[0].m128_f32[2];
    m_planes[5].y = viewProj.r[1].m128_f32[3] - viewProj.r[1].m128_f32[2];
    m_planes[5].z = viewProj.r[2].m128_f32[3] - viewProj.r[2].m128_f32[2];
    m_planes[5].w = viewProj.r[3].m128_f32[3] - viewProj.r[3].m128_f32[2];
    NormalizePlane(5);
  }

  void Frustum::NormalizePlane(const int i)
  {
    const float mag = std::sqrt(m_planes[i].x * m_planes[i].x +
      m_planes[i].y * m_planes[i].y +
      m_planes[i].z * m_planes[i].z);
    if (mag > 0.00001f) {
      m_planes[i].x /= mag;
      m_planes[i].y /= mag;
      m_planes[i].z /= mag;
      m_planes[i].w /= mag;
    }
  }

  bool Frustum::CheckPoint(const float x, const float y, const float z) const
  {
    for (auto p : m_planes) if ((p.x * x) + (p.y * y) + (p.z * z) + p.w < 0.0f) return false;
    return true;
  }

  bool Frustum::CheckSphere(const float x, const float y, const float z, const float r) const
  {
    for (auto m_plane : m_planes) {
      float d = (m_plane.x * x) + (m_plane.y * y) + (m_plane.z * z) + m_plane.w;
      if (d < -r) return false;
    }
    return true;
  }

  bool Frustum::CheckCube(const float x, const float y, const float z, const float r) const
  {
    for (auto m_plane : m_planes) {
      // On vérifie si au moins un point du cube est dans le côté positif du plan
      if ((m_plane.x * (x - r) + m_plane.y * (y - r) + m_plane.z * (z - r) + m_plane.w) >=
        0) continue;
      if ((m_plane.x * (x + r) + m_plane.y * (y - r) + m_plane.z * (z - r) + m_plane.w) >=
        0) continue;
      if ((m_plane.x * (x - r) + m_plane.y * (y + r) + m_plane.z * (z - r) + m_plane.w) >=
        0) continue;
      if ((m_plane.x * (x + r) + m_plane.y * (y + r) + m_plane.z * (z - r) + m_plane.w) >=
        0) continue;
      if ((m_plane.x * (x - r) + m_plane.y * (y - r) + m_plane.z * (z + r) + m_plane.w) >=
        0) continue;
      if ((m_plane.x * (x + r) + m_plane.y * (y - r) + m_plane.z * (z + r) + m_plane.w) >=
        0) continue;
      if ((m_plane.x * (x - r) + m_plane.y * (y + r) + m_plane.z * (z + r) + m_plane.w) >=
        0) continue;
      if ((m_plane.x * (x + r) + m_plane.y * (y + r) + m_plane.z * (z + r) + m_plane.w) >=
        0) continue;

      return false;
    }
    return true;
  }

  bool Frustum::CheckRectangle(const float x,
                               const float y,
                               const float z,
                               const float xSize,
                               const float ySize,
                               const float zSize) const
  {
    // Même logique que CheckCube, mais pour un rectangle
    for (auto m_plane : m_planes) {
      if ((m_plane.x * (x - xSize) + m_plane.y * (y - ySize) + m_plane.z * (z - zSize) + m_plane.w)
        >= 0) continue;
      if ((m_plane.x * (x + xSize) + m_plane.y * (y - ySize) + m_plane.z * (z - zSize) + m_plane.w)
        >= 0) continue;
      if ((m_plane.x * (x - xSize) + m_plane.y * (y + ySize) + m_plane.z * (z - zSize) + m_plane.w)
        >= 0) continue;
      if ((m_plane.x * (x + xSize) + m_plane.y * (y + ySize) + m_plane.z * (z - zSize) + m_plane.w)
        >= 0) continue;
      if ((m_plane.x * (x - xSize) + m_plane.y * (y - ySize) + m_plane.z * (z + zSize) + m_plane.w)
        >= 0) continue;
      if ((m_plane.x * (x + xSize) + m_plane.y * (y - ySize) + m_plane.z * (z + zSize) + m_plane.w)
        >= 0) continue;
      if ((m_plane.x * (x - xSize) + m_plane.y * (y + ySize) + m_plane.z * (z + zSize) + m_plane.w)
        >= 0) continue;
      if ((m_plane.x * (x + xSize) + m_plane.y * (y + ySize) + m_plane.z * (z + zSize) + m_plane.w)
        >= 0) continue;

      return false;
    }
    return true;
  }

  bool Frustum::CheckBox(const AABB& box) const
  {
    const XMFLOAT3 c = box.Center();
    const XMFLOAT3 e = box.Extents();
    return CheckRectangle(c.x, c.y, c.z, e.x, e.y, e.z);
  }

  void Frustum::ConstructFrustumFromMatrix(const XMMATRIX &viewProj)
  {
    // Left plane
    m_planes[0].x = viewProj.r[0].m128_f32[3] + viewProj.r[0].m128_f32[0];
    m_planes[0].y = viewProj.r[1].m128_f32[3] + viewProj.r[1].m128_f32[0];
    m_planes[0].z = viewProj.r[2].m128_f32[3] + viewProj.r[2].m128_f32[0];
    m_planes[0].w = viewProj.r[3].m128_f32[3] + viewProj.r[3].m128_f32[0];
    NormalizePlane(0);

    // Right plane
    m_planes[1].x = viewProj.r[0].m128_f32[3] - viewProj.r[0].m128_f32[0];
    m_planes[1].y = viewProj.r[1].m128_f32[3] - viewProj.r[1].m128_f32[0];
    m_planes[1].z = viewProj.r[2].m128_f32[3] - viewProj.r[2].m128_f32[0];
    m_planes[1].w = viewProj.r[3].m128_f32[3] - viewProj.r[3].m128_f32[0];
    NormalizePlane(1);

    // Bottom plane
    m_planes[2].x = viewProj.r[0].m128_f32[3] + viewProj.r[0].m128_f32[1];
    m_planes[2].y = viewProj.r[1].m128_f32[3] + viewProj.r[1].m128_f32[1];
    m_planes[2].z = viewProj.r[2].m128_f32[3] + viewProj.r[2].m128_f32[1];
    m_planes[2].w = viewProj.r[3].m128_f32[3] + viewProj.r[3].m128_f32[1];
    NormalizePlane(2);

    // Top plane
    m_planes[3].x = viewProj.r[0].m128_f32[3] - viewProj.r[0].m128_f32[1];
    m_planes[3].y = viewProj.r[1].m128_f32[3] - viewProj.r[1].m128_f32[1];
    m_planes[3].z = viewProj.r[2].m128_f32[3] - viewProj.r[2].m128_f32[1];
    m_planes[3].w = viewProj.r[3].m128_f32[3] - viewProj.r[3].m128_f32[1];
    NormalizePlane(3);

    // Near plane
    m_planes[4].x = viewProj.r[0].m128_f32[2];
    m_planes[4].y = viewProj.r[1].m128_f32[2];
    m_planes[4].z = viewProj.r[2].m128_f32[2];
    m_planes[4].w = viewProj.r[3].m128_f32[2];
    NormalizePlane(4);

    // Far plane
    m_planes[5].x = viewProj.r[0].m128_f32[3] - viewProj.r[0].m128_f32[2];
    m_planes[5].y = viewProj.r[1].m128_f32[3] - viewProj.r[1].m128_f32[2];
    m_planes[5].z = viewProj.r[2].m128_f32[3] - viewProj.r[2].m128_f32[2];
    m_planes[5].w = viewProj.r[3].m128_f32[3] - viewProj.r[3].m128_f32[2];
    NormalizePlane(5);
  }

}
