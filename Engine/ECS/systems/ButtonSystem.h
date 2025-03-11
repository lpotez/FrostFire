#pragma once
#include "ECS/components/ButtonSoundComponent.h"
#include "Engine/ECS/core/World.h"
#include "ECS/components/ButtonComponent.h"
#include "ECS/core/System.h"

namespace FrostFireEngine
{
  class ButtonSystem : public System {
  public:
    ButtonSystem() = default;

    void Update(float deltaTime) override
    {


      UNREFERENCED_PARAMETER(deltaTime);

      const auto& world = World::GetInstance();
      const auto  entities = world.GetEntitiesWith<ButtonComponent>();
      const auto  entitiesSound = world.GetEntitiesWith<ButtonSoundComponent>();

      for (const auto& entity : entities) {
        if (auto* button = world.GetEntity(entity->GetId())->GetComponent<ButtonComponent>()) {
          button->Update();
        }
      }

      for (const auto &entity : entitiesSound) {
        if (auto *button = world.GetEntity(entity->GetId())->GetComponent<ButtonSoundComponent>()) {
          button->Update();
        }
      }
    }

    void Initialize() override
    {
    }
    void Cleanup() override
    {
    }

    static ButtonSystem& Get()
    {
      auto* system = World::GetInstance().GetSystem<ButtonSystem>();
      if (!system) {
        throw std::runtime_error("ButtonSystem not initialized");
      }
      return *system;
    }
  };
}
