#pragma once
#include <DirectXMath.h>

namespace FrostFireEngine
{
    struct FBXMaterial
    {
        DirectX::XMFLOAT4 ambient{1.0f, 1.0f, 1.0f, 1.0f};
        DirectX::XMFLOAT4 diffuse{1.0f, 1.0f, 1.0f, 1.0f};
        DirectX::XMFLOAT4 specular{1.0f, 1.0f, 1.0f, 1.0f};
        float specularPower{32.0f};
        float opacity{1.0f};
        std::wstring diffuseMap;
        std::wstring normalMap;
        std::wstring occlusionMap;

        FBXMaterial() = default;
    };

    struct FBXNodeTransform
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 rotation;
        DirectX::XMFLOAT3 scale;

        FBXNodeTransform()
            : position(0.0f, 0.0f, 0.0f)
              , rotation(0.0f, 0.0f, 0.0f)
              , scale(1.0f, 1.0f, 1.0f)
        {
        }

        FBXNodeTransform(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& rot, const DirectX::XMFLOAT3& scl)
            : position(pos)
              , rotation(rot)
              , scale(scl)
        {
        }
    };
}
