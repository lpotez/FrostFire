#pragma once
#include <span>
#include <memory>

#include "Entity.h"

namespace FrostFireEngine
{
  enum class SystemPhase {
    Input,
    Physics,
    Logic,
    Rendering,
    Count
  };

  class System {
  public:
    virtual ~System() = default;

  protected:
    friend class World;
    virtual void Update(float deltaTime) = 0;

    virtual void Initialize()
    {
    }
    virtual void Cleanup()
    {
    }

    template <typename... Components>
    [[nodiscard]] std::span<const std::shared_ptr<Entity>> GetEntitiesWith() const
    {
      return World::GetInstance().GetEntitiesWith<Components...>();
    }
  };
}
