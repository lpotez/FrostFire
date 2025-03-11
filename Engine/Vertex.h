#pragma once
#include <DirectXMath.h>
#include <d3d11.h>

namespace FrostFireEngine
{
    using namespace DirectX;

    class Vertex
    {
    public:
        Vertex() = default;

        Vertex(const XMFLOAT3& position, const XMFLOAT3& normal,
               const XMFLOAT2& texCoord, const XMFLOAT3& tangent = XMFLOAT3(1.0f, 0.0f, 0.0f))
            : m_Position(position)
              , m_Normal(normal)
              , m_TexCoord(texCoord)
              , m_Tangent(tangent)
        {
        }

        static const D3D11_INPUT_ELEMENT_DESC layout[];
        static const UINT numElements;

        const XMFLOAT3& GetPosition() const { return m_Position; }
        const XMFLOAT3& GetNormal() const { return m_Normal; }
        const XMFLOAT2& GetTexCoord() const { return m_TexCoord; }
        const XMFLOAT3& GetTangent() const { return m_Tangent; }

        void SetPosition(const XMFLOAT3& position) { m_Position = position; }
        void SetNormal(const XMFLOAT3& normal) { m_Normal = normal; }
        void SetTexCoord(const XMFLOAT2& texCoord) { m_TexCoord = texCoord; }
        void SetTangent(const XMFLOAT3& tangent) { m_Tangent = tangent; }

    private:
        XMFLOAT3 m_Position;
        XMFLOAT3 m_Normal;
        XMFLOAT2 m_TexCoord;
        XMFLOAT3 m_Tangent;
    };

    inline const D3D11_INPUT_ELEMENT_DESC Vertex::layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    inline const UINT Vertex::numElements = 4;
}
