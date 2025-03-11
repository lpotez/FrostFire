#pragma once
#include "Engine/ECS/core/World.h"
#include "Engine/ECS/core/System.h"
#include "Engine/ECS/components/SliderComponent.h"

namespace FrostFireEngine
{
  class SliderSystem : public System {
  public:
    SliderSystem() = default;

    void Update(float deltaTime) override
    {


      UNREFERENCED_PARAMETER(deltaTime);

      const auto &world = World::GetInstance();
      const auto  entities = world.GetEntitiesWith<SliderComponent>();

      for (const auto &entity : entities) {
        if (auto *slider = world.GetEntity(entity->GetId())->GetComponent<SliderComponent>()) {
          slider->Update();
        }
      }
    }

    void Initialize() override
    {
    }
    void Cleanup() override
    {
    }

    static SliderSystem &Get()
    {
      auto *system = World::GetInstance().GetSystem<SliderSystem>();
      if (!system) {
        throw std::runtime_error("SliderSystem not initialized");
      }
      return *system;
    }
  };
}
