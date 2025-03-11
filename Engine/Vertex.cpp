#include "stdafx.h"
#include "Vertex.h"

namespace PM3D
{
    const UINT Vertex::numElements = 3;

    const D3D11_INPUT_ELEMENT_DESC Vertex::layout[] =
    {
        // Position
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, m_Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        // Normal
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, m_Normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        // Texture Coordinate
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, m_TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
}
