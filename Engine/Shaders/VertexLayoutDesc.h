#pragma once
#include <d3d11.h>
#include <vector>

namespace FrostFireEngine
{
  struct VertexLayoutDesc {
    std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
  };
}
