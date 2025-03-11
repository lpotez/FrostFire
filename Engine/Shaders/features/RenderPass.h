#pragma once
#include <cstdint>

namespace FrostFireEngine
{
  enum class RenderPass : uint16_t {
    None = 0,
    Shadow = 1 << 0,
    GBuffer = 1 << 1,
    Lighting = 1 << 2,
    Skybox = 1 << 3,
    Transparency = 1 << 4,
    PostProcess = 1 << 5,
    Final = 1 << 6,
    UI = 1 << 7,
    Debug = 1 << 8,
  };
}
