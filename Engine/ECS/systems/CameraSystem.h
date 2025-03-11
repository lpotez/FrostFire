#pragma once
#include "Engine/ECS/core/World.h"
#include "Engine/ECS/components/CameraComponent.h"
#include "Engine/ECS/components/transform/TransformComponent.h"

namespace FrostFireEngine
{
  class CameraSystem : public System {
  public:
    CameraSystem() = default;

    void Initialize() override
    {
      // No specific initialization required
    }

    void Update(float deltaTime) override
    {
      // No per-frame updates required for the camera system itself
      UNREFERENCED_PARAMETER(deltaTime);
    }

    void Cleanup() override
    {
      // No specific cleanup required
    }

    void SetActiveCamera(const EntityId cameraId)
    {
      const World& world = World::GetInstance();

      if (const auto cameraEntity = world.GetEntity(cameraId); cameraEntity && cameraEntity->
        HasComponent<CameraComponent>()) {
        activeCameraEntity_ = cameraEntity;
      }
      else {
        throw std::runtime_error(
          "Invalid camera entity ID or entity does not have a CameraComponent.");
      }
    }

    std::shared_ptr<Entity> GetActiveCameraEntity() const
    {
      return activeCameraEntity_;
    }

    CameraComponent* GetActiveCameraComponent() const
    {
      return activeCameraEntity_ ? activeCameraEntity_->GetComponent<CameraComponent>() : nullptr;
    }

    TransformComponent* GetActiveCameraTransform() const
    {
      return activeCameraEntity_
               ? activeCameraEntity_->GetComponent<TransformComponent>()
               : nullptr;
    }

    static CameraSystem& Get()
    {
      auto* system = World::GetInstance().GetSystem<CameraSystem>();
      if (!system) {
        throw std::runtime_error("CameraSystem not initialized");
      }
      return *system;
    }

  private:
    std::shared_ptr<Entity> activeCameraEntity_;
  };
} // namespace FrostFireEngine
