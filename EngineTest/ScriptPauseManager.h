#pragma once
#include "InputManager.h"
#include "ECS/components/rendering/TextRendererComponent.h"
#include "Engine/ECS/core/World.h"

#include "ECS/components/rendering/UIRendererComponent.h"
#include "ECS/systems/PauseManagerSystem.h"
#include "ECS/systems/PhysicsSystem.h"


namespace FrostFireEngine
{
  class ScriptPauseManager final : public ScriptComponent {
    EntityId             pauseMenuUI;
    UIRendererComponent* uiRendererComponent = nullptr;
    bool                 currentState;
    bool                 isPauseKeyPressed = false;

  public:
    explicit ScriptPauseManager(EntityId pauseMenuUI) : pauseMenuUI(pauseMenuUI),
                                                        currentState(false)
    {
    }

    void Awake() override
    {
      uiRendererComponent = World::GetInstance().GetEntity(pauseMenuUI)->GetComponent<
        UIRendererComponent>();
      currentState = IsPaused();
    }

    void Start() override
    {
      currentState = IsPaused();
    }

    void Update(float /*deltaTime*/) override
    {
      if (InputManager::GetInstance().IsKeyPressed(DIK_ESCAPE)) {
        if (!isPauseKeyPressed) {
          isPauseKeyPressed = true;
          // Inverser l'état de pause
          currentState = !currentState;

          // Mettre à jour le PauseManagerSystem
          PauseManagerSystem::Get().SetPaused(currentState);

          InputManager::GetInstance().SetUIMode(currentState);

          // Mettre à jour l'affichage du UI
          UpdateVisibleRecursive(*(World::GetInstance().GetEntity(pauseMenuUI))); 
        }
      }
      else {
        isPauseKeyPressed = false;
      }
    }

    static bool IsPaused()
    {
      return PauseManagerSystem::Get().IsPaused();
    }

    bool GetCurrentState()
    {
      return currentState;
    }

    void UpdateVisibleRecursive(Entity& entity) 
    {
      if (!World::GetInstance().GetEntity(pauseMenuUI)) return;

      // Mise à jour pour les UIRenderer
      if (auto *component = entity.GetComponent<UIRendererComponent>())
      {
        component->SetVisible(currentState);
      }

      // Mise à jour pour les TextRenderer
      if (auto *component = entity.GetComponent<TextRendererComponent>())
      {
        component->SetVisible(currentState);
      }

      // Mise à jour pour les enfants
      if (auto *transform = entity.GetComponent<TransformComponent>())
      {
        for (const auto &childId : transform->GetChildren())
        {
          if (auto childEntity = World::GetInstance().GetEntity(childId)) 
          {
            UpdateVisibleRecursive(*childEntity);
          }
        }
      }
    }
  };
}
