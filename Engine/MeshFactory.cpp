#include "stdafx.h"
#include "MeshFactory.h"
#include "Vertex.h"
#include <DirectXMath.h>
#include <vector>
#include <cmath>
#include <stdexcept>

namespace FrostFireEngine
{
  using namespace DirectX;

  std::shared_ptr<Mesh> MeshFactory::CreateCube(ID3D11Device* device, float halfSize)
  {
    std::vector<Vertex> vertices = {
      Vertex(XMFLOAT3(-halfSize, halfSize, -halfSize), XMFLOAT3(0.0f, 0.0f, -1.0f),
             XMFLOAT2(0.0f, 0.0f),
             XMFLOAT3(1.0f, 0.0f, 0.0f)),
      Vertex(XMFLOAT3(halfSize, halfSize, -halfSize), XMFLOAT3(0.0f, 0.0f, -1.0f),
             XMFLOAT2(1.0f, 0.0f),
             XMFLOAT3(1.0f, 0.0f, 0.0f)),
      Vertex(XMFLOAT3(halfSize, -halfSize, -halfSize), XMFLOAT3(0.0f, 0.0f, -1.0f),
             XMFLOAT2(1.0f, 1.0f),
             XMFLOAT3(1.0f, 0.0f, 0.0f)),
      Vertex(XMFLOAT3(-halfSize, -halfSize, -halfSize), XMFLOAT3(0.0f, 0.0f, -1.0f),
             XMFLOAT2(0.0f, 1.0f),
             XMFLOAT3(1.0f, 0.0f, 0.0f)),

      Vertex(XMFLOAT3(halfSize, halfSize, halfSize), XMFLOAT3(0.0f, 0.0f, 1.0f),
             XMFLOAT2(0.0f, 0.0f),
             XMFLOAT3(-1.0f, 0.0f, 0.0f)),
      Vertex(XMFLOAT3(-halfSize, halfSize, halfSize), XMFLOAT3(0.0f, 0.0f, 1.0f),
             XMFLOAT2(1.0f, 0.0f),
             XMFLOAT3(-1.0f, 0.0f, 0.0f)),
      Vertex(XMFLOAT3(-halfSize, -halfSize, halfSize), XMFLOAT3(0.0f, 0.0f, 1.0f),
             XMFLOAT2(1.0f, 1.0f),
             XMFLOAT3(-1.0f, 0.0f, 0.0f)),
      Vertex(XMFLOAT3(halfSize, -halfSize, halfSize), XMFLOAT3(0.0f, 0.0f, 1.0f),
             XMFLOAT2(0.0f, 1.0f),
             XMFLOAT3(-1.0f, 0.0f, 0.0f)),

      Vertex(XMFLOAT3(-halfSize, halfSize, halfSize), XMFLOAT3(-1.0f, 0.0f, 0.0f),
             XMFLOAT2(0.0f, 0.0f),
             XMFLOAT3(0.0f, 0.0f, -1.0f)),
      Vertex(XMFLOAT3(-halfSize, halfSize, -halfSize), XMFLOAT3(-1.0f, 0.0f, 0.0f),
             XMFLOAT2(1.0f, 0.0f),
             XMFLOAT3(0.0f, 0.0f, -1.0f)),
      Vertex(XMFLOAT3(-halfSize, -halfSize, -halfSize), XMFLOAT3(-1.0f, 0.0f, 0.0f),
             XMFLOAT2(1.0f, 1.0f),
             XMFLOAT3(0.0f, 0.0f, -1.0f)),
      Vertex(XMFLOAT3(-halfSize, -halfSize, halfSize), XMFLOAT3(-1.0f, 0.0f, 0.0f),
             XMFLOAT2(0.0f, 1.0f),
             XMFLOAT3(0.0f, 0.0f, -1.0f)),

      Vertex(XMFLOAT3(halfSize, halfSize, -halfSize), XMFLOAT3(1.0f, 0.0f, 0.0f),
             XMFLOAT2(0.0f, 0.0f),
             XMFLOAT3(0.0f, 0.0f, 1.0f)),
      Vertex(XMFLOAT3(halfSize, halfSize, halfSize), XMFLOAT3(1.0f, 0.0f, 0.0f),
             XMFLOAT2(1.0f, 0.0f),
             XMFLOAT3(0.0f, 0.0f, 1.0f)),
      Vertex(XMFLOAT3(halfSize, -halfSize, halfSize), XMFLOAT3(1.0f, 0.0f, 0.0f),
             XMFLOAT2(1.0f, 1.0f),
             XMFLOAT3(0.0f, 0.0f, 1.0f)),
      Vertex(XMFLOAT3(halfSize, -halfSize, -halfSize), XMFLOAT3(1.0f, 0.0f, 0.0f),
             XMFLOAT2(0.0f, 1.0f),
             XMFLOAT3(0.0f, 0.0f, 1.0f)),

      Vertex(XMFLOAT3(-halfSize, halfSize, -halfSize), XMFLOAT3(0.0f, 1.0f, 0.0f),
             XMFLOAT2(0.0f, 0.0f),
             XMFLOAT3(1.0f, 0.0f, 0.0f)),
      Vertex(XMFLOAT3(halfSize, halfSize, -halfSize), XMFLOAT3(0.0f, 1.0f, 0.0f),
             XMFLOAT2(1.0f, 0.0f),
             XMFLOAT3(1.0f, 0.0f, 0.0f)),
      Vertex(XMFLOAT3(halfSize, halfSize, halfSize), XMFLOAT3(0.0f, 1.0f, 0.0f),
             XMFLOAT2(1.0f, 1.0f),
             XMFLOAT3(1.0f, 0.0f, 0.0f)),
      Vertex(XMFLOAT3(-halfSize, halfSize, halfSize), XMFLOAT3(0.0f, 1.0f, 0.0f),
             XMFLOAT2(0.0f, 1.0f),
             XMFLOAT3(1.0f, 0.0f, 0.0f)),

      Vertex(XMFLOAT3(-halfSize, -halfSize, -halfSize), XMFLOAT3(0.0f, -1.0f, 0.0f),
             XMFLOAT2(0.0f, 1.0f),
             XMFLOAT3(1.0f, 0.0f, 0.0f)),
      Vertex(XMFLOAT3(halfSize, -halfSize, -halfSize), XMFLOAT3(0.0f, -1.0f, 0.0f),
             XMFLOAT2(1.0f, 1.0f),
             XMFLOAT3(1.0f, 0.0f, 0.0f)),
      Vertex(XMFLOAT3(halfSize, -halfSize, halfSize), XMFLOAT3(0.0f, -1.0f, 0.0f),
             XMFLOAT2(1.0f, 0.0f),
             XMFLOAT3(1.0f, 0.0f, 0.0f)),
      Vertex(XMFLOAT3(-halfSize, -halfSize, halfSize), XMFLOAT3(0.0f, -1.0f, 0.0f),
             XMFLOAT2(0.0f, 0.0f),
             XMFLOAT3(1.0f, 0.0f, 0.0f)),
    };

    std::vector<uint32_t> indices = {
      0, 1, 2, 0, 2, 3,
      4, 5, 6, 4, 6, 7,
      8, 9, 10, 8, 10, 11,
      12, 13, 14, 12, 14, 15,
      16, 18, 17, 16, 19, 18,
      20, 21, 22, 20, 22, 23
    };

    return std::make_shared<Mesh>(device, vertices, indices);
  }

  std::shared_ptr<Mesh> MeshFactory::CreateSphere(ID3D11Device* device,
                                                  float         radius,
                                                  uint32_t      sectorCount,
                                                  uint32_t      stackCount)
  {
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    float x,  y,  z, xy;
    float nx, ny, nz;
    float s,  t;

    float sectorStep = 2 * XM_PI / sectorCount;
    float stackStep = XM_PI / stackCount;
    float sectorAngle, stackAngle;

    for (uint32_t i = 0; i <= stackCount; ++i) {
      stackAngle = XM_PI / 2 - i * stackStep;
      float cosStackAngle = cosf(stackAngle);
      float sinStackAngle = sinf(stackAngle);

      xy = radius * cosStackAngle;
      y = radius * sinStackAngle;

      for (uint32_t j = 0; j <= sectorCount; ++j) {
        sectorAngle = j * sectorStep;
        float cosSectorAngle = cosf(sectorAngle);
        float sinSectorAngle = sinf(sectorAngle);

        x = xy * cosSectorAngle;
        z = xy * sinSectorAngle;

        nx = cosStackAngle * cosSectorAngle;
        ny = sinStackAngle;
        nz = cosStackAngle * sinSectorAngle;

        s = static_cast<float>(j) / sectorCount;
        t = static_cast<float>(i) / stackCount;

        float tx = -sinSectorAngle;
        float ty = 0.0f;
        float tz = cosSectorAngle;

        vertices.emplace_back(XMFLOAT3(x, y, z), XMFLOAT3(nx, ny, nz), XMFLOAT2(s, t),
                              XMFLOAT3(tx, ty, tz));
      }
    }

    for (uint32_t i = 0; i < stackCount; ++i) {
      uint32_t k1 = i * (sectorCount + 1);
      uint32_t k2 = k1 + sectorCount + 1;

      for (uint32_t j = 0; j < sectorCount; ++j, ++k1, ++k2) {
        if (i != 0) {
          indices.push_back(k1);
          indices.push_back(k1 + 1);
          indices.push_back(k2);
        }

        if (i != (stackCount - 1)) {
          indices.push_back(k1 + 1);
          indices.push_back(k2 + 1);
          indices.push_back(k2);
        }
      }
    }

    return std::make_shared<Mesh>(device, vertices, indices);
  }


  std::shared_ptr<Mesh> MeshFactory::CreateCubeMap(ID3D11Device* device, float size)
  {
    float half = size / 2.0f;

    // Définition des faces en se basant sur votre convention :
    // +Z = front
    // -Z = back
    // +X = right
    // -X = left
    // +Y = top
    // -Y = bottom
    //
    // Normales pointent vers l'intérieur, donc pour la face front (+Z), la normale intérieure est -Z, etc.
    // Les UV sont présents mais en cube map, ce sont les directions normalisées qui comptent.

    std::vector<Vertex> vertices = {
      // Front (+Z), normale intérieure = -Z
      Vertex(XMFLOAT3(-half, -half, half), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 1)),
      Vertex(XMFLOAT3(-half, half, half), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0)),
      Vertex(XMFLOAT3(half, half, half), XMFLOAT3(0, 0, -1), XMFLOAT2(1, 0)),
      Vertex(XMFLOAT3(half, -half, half), XMFLOAT3(0, 0, -1), XMFLOAT2(1, 1)),

      // Back (-Z), normale intérieure = +Z
      Vertex(XMFLOAT3(half, -half, -half), XMFLOAT3(0, 0, 1), XMFLOAT2(0, 1)),
      Vertex(XMFLOAT3(half, half, -half), XMFLOAT3(0, 0, 1), XMFLOAT2(0, 0)),
      Vertex(XMFLOAT3(-half, half, -half), XMFLOAT3(0, 0, 1), XMFLOAT2(1, 0)),
      Vertex(XMFLOAT3(-half, -half, -half), XMFLOAT3(0, 0, 1), XMFLOAT2(1, 1)),

      // Right (+X), normale intérieure = -X
      Vertex(XMFLOAT3(half, -half, -half), XMFLOAT3(-1, 0, 0), XMFLOAT2(0, 1)),
      Vertex(XMFLOAT3(half, -half, half), XMFLOAT3(-1, 0, 0), XMFLOAT2(1, 1)),
      Vertex(XMFLOAT3(half, half, half), XMFLOAT3(-1, 0, 0), XMFLOAT2(1, 0)),
      Vertex(XMFLOAT3(half, half, -half), XMFLOAT3(-1, 0, 0), XMFLOAT2(0, 0)),

      // Left (-X), normale intérieure = +X
      Vertex(XMFLOAT3(-half, -half, half), XMFLOAT3(1, 0, 0), XMFLOAT2(0, 1)),
      Vertex(XMFLOAT3(-half, -half, -half), XMFLOAT3(1, 0, 0), XMFLOAT2(1, 1)),
      Vertex(XMFLOAT3(-half, half, -half), XMFLOAT3(1, 0, 0), XMFLOAT2(1, 0)),
      Vertex(XMFLOAT3(-half, half, half), XMFLOAT3(1, 0, 0), XMFLOAT2(0, 0)),

      // Top (+Y), normale intérieure = -Y
      Vertex(XMFLOAT3(-half, half, half), XMFLOAT3(0, -1, 0), XMFLOAT2(0, 0)),
      // Modifié l'ordre des vertex
      Vertex(XMFLOAT3(-half, half, -half), XMFLOAT3(0, -1, 0), XMFLOAT2(0, 1)),
      Vertex(XMFLOAT3(half, half, -half), XMFLOAT3(0, -1, 0), XMFLOAT2(1, 1)),
      Vertex(XMFLOAT3(half, half, half), XMFLOAT3(0, -1, 0), XMFLOAT2(1, 0)),

      // Bottom (-Y), normale intérieure = +Y
      Vertex(XMFLOAT3(-half, -half, -half), XMFLOAT3(0, 1, 0), XMFLOAT2(0, 0)),
      // Modifié l'ordre des vertex
      Vertex(XMFLOAT3(-half, -half, half), XMFLOAT3(0, 1, 0), XMFLOAT2(0, 1)),
      Vertex(XMFLOAT3(half, -half, half), XMFLOAT3(0, 1, 0), XMFLOAT2(1, 1)),
      Vertex(XMFLOAT3(half, -half, -half), XMFLOAT3(0, 1, 0), XMFLOAT2(1, 0))
    };

    std::vector<uint32_t> indices = {
      // Front (+Z)
      0, 1, 2, 0, 2, 3,
      // Back (-Z)
      4, 5, 6, 4, 6, 7,
      // Right (+X)
      8, 9, 10, 8, 10, 11,
      // Left (-X)
      12, 13, 14, 12, 14, 15,
      // Top (+Y)
      16, 17, 18, 16, 18, 19,
      // Bottom (-Y)
      20, 21, 22, 20, 22, 23
    };

    return std::make_shared<Mesh>(device, vertices, indices);
  }
  std::shared_ptr<Mesh> MeshFactory::CreatePlane(ID3D11Device* device, float width, float depth)
  {
    float halfWidth = width / 2.0f;
    float halfDepth = depth / 2.0f;

    std::vector<Vertex> vertices = {
      Vertex(
        XMFLOAT3(-halfWidth, 0.0f, -halfDepth),
        XMFLOAT3(0.0f, 1.0f, 0.0f),
        XMFLOAT2(0.0f, 0.0f),
        XMFLOAT3(1.0f, 0.0f, 0.0f)
      ),
      Vertex(
        XMFLOAT3(halfWidth, 0.0f, -halfDepth),
        XMFLOAT3(0.0f, 1.0f, 0.0f),
        XMFLOAT2(1.0f, 0.0f),
        XMFLOAT3(1.0f, 0.0f, 0.0f)
      ),
      Vertex(
        XMFLOAT3(halfWidth, 0.0f, halfDepth),
        XMFLOAT3(0.0f, 1.0f, 0.0f),
        XMFLOAT2(1.0f, 1.0f),
        XMFLOAT3(1.0f, 0.0f, 0.0f)
      ),
      Vertex(
        XMFLOAT3(-halfWidth, 0.0f, halfDepth),
        XMFLOAT3(0.0f, 1.0f, 0.0f),
        XMFLOAT2(0.0f, 1.0f),
        XMFLOAT3(1.0f, 0.0f, 0.0f)
      ),
    };

    std::vector<uint32_t> indices = {
      0, 2, 1,
      0, 3, 2
    };

    return std::make_shared<Mesh>(device, vertices, indices);
  }
  std::shared_ptr<Mesh> MeshFactory::CreateQuad(ID3D11Device* device)
  {
    float               halfSize = 0.5f; // Taille par défaut du Quad : 1.0f x 1.0f
    std::vector<Vertex> vertices = {
      Vertex(XMFLOAT3(-halfSize, -halfSize, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f),
             XMFLOAT2(0.0f, 1.0f)),
      Vertex(XMFLOAT3(-halfSize, halfSize, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)),
      Vertex(XMFLOAT3(halfSize, halfSize, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)),
      Vertex(XMFLOAT3(halfSize, -halfSize, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)),
    };

    std::vector<uint32_t> indices = {
      0, 1, 2,
      0, 2, 3
    };

    return std::make_shared<Mesh>(device, vertices, indices);
  }

  std::shared_ptr<Mesh> MeshFactory::CreateCapsule(ID3D11Device* device,
                                                   float         radius,
                                                   float         halfHeight,
                                                   uint32_t      sectorCount)
  {
    if (sectorCount < 3) {
      throw std::invalid_argument("Le nombre de secteurs doit être au moins 3.");
    }

    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    float sectorStep = 2 * XM_PI / static_cast<float>(sectorCount);

    for (uint32_t i = 0; i <= sectorCount; ++i) {
      float sectorAngle = i * sectorStep;
      float y = radius * cosf(sectorAngle);
      float z = radius * sinf(sectorAngle);
      float ny = cosf(sectorAngle);
      float nz = sinf(sectorAngle);

      float ty = -sinf(sectorAngle);
      float tz = cosf(sectorAngle);

      vertices.emplace_back(
        XMFLOAT3(-halfHeight, y, z),
        XMFLOAT3(0.0f, ny, nz),
        XMFLOAT2(static_cast<float>(i) / sectorCount, 1.0f),
        XMFLOAT3(0.0f, ty, tz)
      );

      vertices.emplace_back(
        XMFLOAT3(halfHeight, y, z),
        XMFLOAT3(0.0f, ny, nz),
        XMFLOAT2(static_cast<float>(i) / sectorCount, 0.0f),
        XMFLOAT3(0.0f, ty, tz)
      );
    }

    for (uint32_t i = 0; i < sectorCount; ++i) {
      uint32_t k1 = i * 2;
      uint32_t k2 = k1 + 1;
      uint32_t k3 = k1 + 2;
      uint32_t k4 = k1 + 3;

      indices.push_back(k1);
      indices.push_back(k2);
      indices.push_back(k3);

      indices.push_back(k3);
      indices.push_back(k2);
      indices.push_back(k4);
    }

    uint32_t stackCount = sectorCount / 2;
    float    stackStep = XM_PI / 2 / stackCount;
    uint32_t baseIndex = static_cast<uint32_t>(vertices.size());

    for (uint32_t i = 0; i <= stackCount; ++i) {
      float stackAngle = i * stackStep;
      float xz = radius * cosf(stackAngle);
      float x = radius * sinf(stackAngle) + halfHeight;

      for (uint32_t j = 0; j <= sectorCount; ++j) {
        float sectorAngle = j * sectorStep;
        float y = xz * cosf(sectorAngle);
        float z = xz * sinf(sectorAngle);

        float nx = sinf(stackAngle);
        float ny = cosf(stackAngle) * cosf(sectorAngle);
        float nz = cosf(stackAngle) * sinf(sectorAngle);

        float ty = -sinf(sectorAngle);
        float tz = cosf(sectorAngle);
        float tx = 0.0f;
        float tangentLength = sqrtf(ty * ty + tz * tz);
        if (tangentLength > 0.0f) {
          ty /= tangentLength;
          tz /= tangentLength;
        }

        vertices.emplace_back(
          XMFLOAT3(x, y, z),
          XMFLOAT3(nx, ny, nz),
          XMFLOAT2(static_cast<float>(j) / sectorCount, static_cast<float>(i) / stackCount),
          XMFLOAT3(tx, ty, tz)
        );
      }
    }

    for (uint32_t i = 0; i < stackCount; ++i) {
      uint32_t k1 = baseIndex + i * (sectorCount + 1);
      uint32_t k2 = k1 + sectorCount + 1;

      for (uint32_t j = 0; j < sectorCount; ++j) {
        indices.push_back(k1 + j);
        indices.push_back(k2 + j);
        indices.push_back(k1 + j + 1);

        indices.push_back(k1 + j + 1);
        indices.push_back(k2 + j);
        indices.push_back(k2 + j + 1);
      }
    }

    baseIndex = static_cast<uint32_t>(vertices.size());
    for (uint32_t i = 0; i <= stackCount; ++i) {
      float stackAngle = XM_PI / 2 - i * stackStep;
      float xz = radius * cosf(stackAngle);
      float x = -(radius * sinf(stackAngle) + halfHeight);

      for (uint32_t j = 0; j <= sectorCount; ++j) {
        float sectorAngle = j * sectorStep;
        float y = xz * cosf(sectorAngle);
        float z = xz * sinf(sectorAngle);

        float nx = -sinf(stackAngle);
        float ny = cosf(stackAngle) * cosf(sectorAngle);
        float nz = cosf(stackAngle) * sinf(sectorAngle);

        float ty = -sinf(sectorAngle);
        float tz = cosf(sectorAngle);
        float tx = 0.0f;
        float tangentLength = sqrtf(ty * ty + tz * tz);
        if (tangentLength > 0.0f) {
          ty /= tangentLength;
          tz /= tangentLength;
        }

        vertices.emplace_back(
          XMFLOAT3(x, y, z),
          XMFLOAT3(nx, ny, nz),
          XMFLOAT2(static_cast<float>(j) / sectorCount, static_cast<float>(i) / stackCount),
          XMFLOAT3(tx, ty, tz)
        );
      }
    }

    for (uint32_t i = 0; i < stackCount; ++i) {
      uint32_t k1 = baseIndex + i * (sectorCount + 1);
      uint32_t k2 = k1 + sectorCount + 1;

      for (uint32_t j = 0; j < sectorCount; ++j) {
        indices.push_back(k1 + j);
        indices.push_back(k2 + j);
        indices.push_back(k1 + j + 1);

        indices.push_back(k1 + j + 1);
        indices.push_back(k2 + j);
        indices.push_back(k2 + j + 1);
      }
    }

    return std::make_shared<Mesh>(device, vertices, indices);
  }
}
