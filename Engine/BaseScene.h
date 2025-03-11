#pragma once
#include "Scene.h"
#include "ECS/systems/ScriptSystem.h"
#include "ECS/systems/PhysicsSystem.h"
#include "ECS/systems/CameraSystem.h"
#include "ECS/systems/ButtonSystem.h"
#include "ECS/systems/LightSystem.h"
#include "ECS/systems/RenderingSystem.h"
#include "ECS/systems/PauseManagerSystem.h"
#include "ECS/systems/SliderSystem.h"
#include "ECS/systems/debug/DebugSystem.h"

namespace FrostFireEngine
{
  class BaseScene : public Scene {
  public:
    void Initialize(DispositifD3D11* pDevice) override
    {
      World& world = GetWorld();

      world.AddSystem<ScriptSystem>(SystemPhase::Logic);
      world.AddSystem<PhysicsSystem>(SystemPhase::Physics);
      world.AddSystem<LightSystem>(SystemPhase::Logic, pDevice->GetD3DDevice());
      world.AddSystem<RenderingSystem>(SystemPhase::Rendering, pDevice);
      world.AddSystem<ButtonSystem>(SystemPhase::Logic);
      world.AddSystem<SliderSystem>(SystemPhase::Logic);
      world.AddSystem<AudioSystem>(SystemPhase::Logic);
      world.AddSystem<CameraSystem>(SystemPhase::Logic);
      world.AddSystem<PauseManagerSystem>(SystemPhase::Logic);
      #ifdef _DEBUG
      world.AddSystem<DebugSystem>(SystemPhase::Rendering, pDevice);
      #endif

      SetupCamera();

      world.InitializeSystems();
    }

  protected:
    virtual void SetupCamera()
    {
      World& world = GetWorld();

      auto cameraEntity = world.CreateEntity();

      auto& transform = cameraEntity->AddComponent<TransformComponent>();
      transform.SetPosition({0.0f, 0.0f, -10.0f});

      constexpr float fieldOfView = XM_PI / 4;
      constexpr float aspectRatio = 16.0f / 9.0f;
      constexpr float nearPlane = 0.1f;
      constexpr float farPlane = 1000.0f;

      cameraEntity->AddComponent<CameraComponent>(
        CameraComponent(fieldOfView, aspectRatio, nearPlane, farPlane, cameraEntity->GetId())
      );

      if (auto* cameraSystem = world.GetSystem<CameraSystem>()) {
        cameraSystem->SetActiveCamera(cameraEntity->GetId());
      }
    }
  };
}
