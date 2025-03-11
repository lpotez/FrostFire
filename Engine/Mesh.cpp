#include "stdafx.h"
#include "Mesh.h"
#include "resource.h"
#include "util.h"
#undef min
#undef max

namespace FrostFireEngine
{
  Mesh::Mesh(ID3D11Device*                device,
             const std::vector<Vertex>&   vertices,
             const std::vector<uint32_t>& indices)
    : indexCount(static_cast<UINT>(indices.size()))
      , mVertices(vertices)
      , mIndices(indices)
  {
    CalculateBounds();
    CreateBuffers(device);
  }

  Mesh::~Mesh()
  {
    DXRelacher(pIndexBuffer);
    DXRelacher(pVertexBuffer);
  }

  void Mesh::CalculateBounds()
  {
    if (mVertices.empty()) return;

    const XMFLOAT3 firstPos = mVertices[0].GetPosition();
    XMFLOAT3       minBounds = firstPos;
    XMFLOAT3       maxBounds = firstPos;

    for (size_t i = 1; i < mVertices.size(); ++i) {
      const XMFLOAT3& pos = mVertices[i].GetPosition();
      minBounds.x = std::min(minBounds.x, pos.x);
      minBounds.y = std::min(minBounds.y, pos.y);
      minBounds.z = std::min(minBounds.z, pos.z);

      maxBounds.x = std::max(maxBounds.x, pos.x);
      maxBounds.y = std::max(maxBounds.y, pos.y);
      maxBounds.z = std::max(maxBounds.z, pos.z);
    }

    // On stocke directement dans l'AABB
    mBounds.box.min = minBounds;
    mBounds.box.max = maxBounds;

    // Calcul de la sphère englobante : on utilise le centre fourni par l'AABB
    XMFLOAT3 center = mBounds.box.Center();
    XMVECTOR vCenter = XMLoadFloat3(&center);
    XMStoreFloat3(&mBounds.sphere.center, vCenter);

    float maxRadiusSq = 0.0f;
    for (const auto& vertex : mVertices) {
      XMVECTOR pos = XMLoadFloat3(&vertex.GetPosition());
      XMVECTOR diff = XMVectorSubtract(pos, vCenter);
      float    distSq = XMVectorGetX(XMVector3LengthSq(diff));
      maxRadiusSq = std::max(maxRadiusSq, distSq);
    }

    mBounds.sphere.radius = std::sqrt(maxRadiusSq);
  }

  void Mesh::CreateBuffers(ID3D11Device* device)
  {
    if (!device || mVertices.empty() || mIndices.empty()) return;

    D3D11_BUFFER_DESC vertexBufferDesc{};
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * mVertices.size());
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexData{};
    vertexData.pSysMem = mVertices.data();

    DXEssayer(device->CreateBuffer(&vertexBufferDesc, &vertexData, &pVertexBuffer),
              DXE_CREATIONVERTEXBUFFER);

    D3D11_BUFFER_DESC indexBufferDesc{};
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * mIndices.size());
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexData{};
    indexData.pSysMem = mIndices.data();

    DXEssayer(device->CreateBuffer(&indexBufferDesc, &indexData, &pIndexBuffer),
              DXE_CREATIONINDEXBUFFER);
  }

  void Mesh::Draw(ID3D11DeviceContext* context) const
  {
    if (!context || !pVertexBuffer || !pIndexBuffer) return;

    static constexpr UINT stride = sizeof(Vertex);
    static constexpr UINT offset = 0;

    context->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(indexCount, 0, 0);
  }
}
