#pragma once

#include "Mesh.h"
#include <memory>
#include <string>
#include <stdexcept>

namespace FrostFireEngine
{
    class MeshFactory
    {
    public:
        static std::shared_ptr<Mesh> CreateCube(ID3D11Device* device, float size = 1.0f);
        static std::shared_ptr<Mesh> CreateCubeMap(ID3D11Device* device, float size);
        static std::shared_ptr<Mesh> CreateSphere(ID3D11Device* device, float radius = 1.0f, uint32_t sectorCount = 36,
                                                  uint32_t stackCount = 18);
        static std::shared_ptr<Mesh> CreateCapsule(ID3D11Device* device, float radius, float halfHeight,
                                                   uint32_t sectorCount);
      static std::shared_ptr<Mesh>   CreatePlane(ID3D11Device* device, float width = 1.0f, float depth = 1.0f);
        static std::shared_ptr<Mesh> CreateQuad(ID3D11Device* device);

    };
} // namespace PM3D
