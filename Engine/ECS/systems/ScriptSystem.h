#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <queue>

#include "Engine/ECS/core/World.h"
#include "Engine/ECS/components/scripts/ScriptComponent.h"

namespace FrostFireEngine
{
  class ScriptSystem : public System {
  public:
    ScriptSystem() = default;
    ~ScriptSystem() override = default;

    // Prevent copying to ensure single system instance
    ScriptSystem(const ScriptSystem&) = delete;
    ScriptSystem& operator=(const ScriptSystem&) = delete;

    void Initialize() override
    {
      scripts.reserve(100); // Pre-allocate space for common use case
    }

    void Update(float deltaTime) override
    {
      // Process scripts waiting for Awake
      while (!awakeQueue.empty()) {
        auto* script = awakeQueue.front();
        if (script && !script->HasAwoken() && script->IsEnabled()) {
          script->Awake();
          script->awoken = true;
          startQueue.push(script);
        }
        awakeQueue.pop();
      }

      // Process scripts waiting for Start
      while (!startQueue.empty()) {
        auto* script = startQueue.front();
        if (script && script->HasAwoken() && !script->HasStarted() && script->IsEnabled()) {
          script->Start();
          script->started = true;
        }
        startQueue.pop();
      }

      // Update all active scripts
      for (auto& script : scripts) {
        if (script && script->HasStarted() && script->IsEnabled()) {
          script->Update(deltaTime);
        }
      }
    }

    void Cleanup() override
    {
      scripts.clear();
      while (!awakeQueue.empty()) awakeQueue.pop();
      while (!startQueue.empty()) startQueue.pop();
    }

    // Adds a script to the system and queues it for initialization if enabled
    void RegisterScript(ScriptComponent* script)
    {
      if (std::find(scripts.begin(), scripts.end(), script) == scripts.end()) {
        scripts.emplace_back(script);
        if (script->IsEnabled()) {
          awakeQueue.push(script);
        }
      }
    }

    // Removes a script from the system
    void UnregisterScript(ScriptComponent* script)
    {
      std::erase(scripts, script);
    }

    // Handles script enable state changes
    void OnScriptEnabled(ScriptComponent* script)
    {
      if (script) {
        script->OnEnable();
        if (!script->HasAwoken()) {
          awakeQueue.push(script);
        }
      }
    }

    // Handles script disable state changes
    void OnScriptDisabled(ScriptComponent* script)
    {
      if (script) {
        script->OnDisable();
      }
    }

    // Collision event handlers - notifies relevant scripts
    void OnCollisionEnter(EntityId entityA, EntityId entityB)
    {
      NotifyScripts(entityA, entityB, &ScriptComponent::OnCollisionEnter);
      NotifyScripts(entityB, entityA, &ScriptComponent::OnCollisionEnter);
    }

    void OnCollisionStay(EntityId entityA, EntityId entityB)
    {
      NotifyScripts(entityA, entityB, &ScriptComponent::OnCollisionStay);
      NotifyScripts(entityB, entityA, &ScriptComponent::OnCollisionStay);
    }

    void OnCollisionExit(EntityId entityA, EntityId entityB)
    {
      NotifyScripts(entityA, entityB, &ScriptComponent::OnCollisionExit);
      NotifyScripts(entityB, entityA, &ScriptComponent::OnCollisionExit);
    }

    void OnTriggerEnter(EntityId entityA, EntityId entityB)
    {
      NotifyScripts(entityA, entityB, &ScriptComponent::OnTriggerEnter);
      NotifyScripts(entityB, entityA, &ScriptComponent::OnTriggerEnter);
    }

    void OnTriggerStay(EntityId entityA, EntityId entityB)
    {
      NotifyScripts(entityA, entityB, &ScriptComponent::OnTriggerStay);
      NotifyScripts(entityB, entityA, &ScriptComponent::OnTriggerStay);
    }

    void OnTriggerExit(EntityId entityA, EntityId entityB)
    {
      NotifyScripts(entityA, entityB, &ScriptComponent::OnTriggerExit);
      NotifyScripts(entityB, entityA, &ScriptComponent::OnTriggerExit);
    }

  private:
    std::vector<ScriptComponent*> scripts;
    std::queue<ScriptComponent*>  awakeQueue;
    std::queue<ScriptComponent*>  startQueue;

    // Template method to notify scripts of physics events
    template <typename Func>
    void NotifyScripts(EntityId entity, EntityId other, Func func)
    {
      auto entityPtr = World::GetInstance().GetEntity(entity);
      if (!entityPtr) return;

      for (auto& script : scripts) {
        if (script && script->HasStarted() && script->IsEnabled() && script->GetOwnerId() ==
          entity) {
          (script->*func)(other);
        }
      }
    }
  };
}
