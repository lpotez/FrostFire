#pragma once
#include <cstdint>
#include <limits>
#undef max

namespace FrostFireEngine
{
    using EntityId = std::uint32_t;
    using ComponentMask = std::uint32_t;
    constexpr EntityId INVALID_ENTITY_ID = std::numeric_limits<EntityId>::max();
    constexpr size_t MAX_ENTITIES = 5000;
    constexpr size_t MAX_COMPONENTS = 32; // Optimisé pour uint32_t mask
}
