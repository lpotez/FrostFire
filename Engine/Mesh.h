#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <cfloat>
#include <algorithm>
#include "Vertex.h"
#include "Math/AABB.h"

using namespace DirectX;

namespace FrostFireEngine
{
  class Mesh {
    ID3D11Buffer* pVertexBuffer = nullptr;
    ID3D11Buffer* pIndexBuffer = nullptr;
    UINT          indexCount = 0;

    struct Bounds {
      AABB box;

      struct {
        XMFLOAT3 center;
        float    radius;
      } sphere;
    } mBounds;

    std::vector<Vertex>   mVertices;
    std::vector<uint32_t> mIndices;

  public:
    Mesh(ID3D11Device*                device,
         const std::vector<Vertex>&   vertices,
         const std::vector<uint32_t>& indices);
    ~Mesh();

    void          Draw(ID3D11DeviceContext* context) const;
    ID3D11Buffer* GetVertexBuffer() const
    {
      return pVertexBuffer;
    }
    ID3D11Buffer* GetIndexBuffer() const
    {
      return pIndexBuffer;
    }
    UINT GetIndexCount() const
    {
      return indexCount;
    }

    XMFLOAT3 GetMinBounds() const
    {
      return mBounds.box.min;
    }
    XMFLOAT3 GetMaxBounds() const
    {
      return mBounds.box.max;
    }

    const std::vector<Vertex>& GetVertices() const
    {
      return mVertices;
    }
    const std::vector<uint32_t>& GetIndices() const
    {
      return mIndices;
    }

    const Bounds& GetBounds() const
    {
      return mBounds;
    }

  private:
    void CalculateBounds();
    void CreateBuffers(ID3D11Device* device);
  };
}
