#pragma once
#include "Engine/ECS/core/System.h"
#include "Engine/ECS/core/World.h"

namespace FrostFireEngine
{
  class PauseManagerSystem : public System {
  public:
    PauseManagerSystem() : paused(false)
    {
    }
    ~PauseManagerSystem() override = default;

    void Initialize() override
    {
    }
    void Update(float /*deltaTime*/) override
    {
    }
    void Cleanup() override
    {
    }

    void SetPaused(bool state)
    {
      paused = state;
    }

    bool IsPaused() const
    {
      return paused;
    }

    static PauseManagerSystem& Get()
    {
      auto* system = World::GetInstance().GetSystem<PauseManagerSystem>();
      if (!system) {
        throw std::runtime_error("PauseManagerSystem not initialized");
      }
      return *system;
    }

  private:
    bool paused;
  };
}
