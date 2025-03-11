#pragma once
#include <DirectXMath.h>
using namespace DirectX;

namespace FrostFireEngine
{
  struct AABB {
    XMFLOAT3 min;
    XMFLOAT3 max;

    XMFLOAT3 Center() const
    {
      return XMFLOAT3((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f, (min.z + max.z) * 0.5f);
    }

    XMFLOAT3 Extents() const
    {
      return XMFLOAT3((max.x - min.x) * 0.5f, (max.y - min.y) * 0.5f, (max.z - min.z) * 0.5f);
    }

    bool Contains(const AABB& other) const
    {
      return (other.min.x >= min.x && other.max.x <= max.x &&
        other.min.y >= min.y && other.max.y <= max.y &&
        other.min.z >= min.z && other.max.z <= max.z);
    }

    bool Intersects(const AABB& other) const
    {
      if (other.max.x < min.x || other.min.x > max.x) return false;
      if (other.max.y < min.y || other.min.y > max.y) return false;
      if (other.max.z < min.z || other.min.z > max.z) return false;
      return true;
    }
  };
}
