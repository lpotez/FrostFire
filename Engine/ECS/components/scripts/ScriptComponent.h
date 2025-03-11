#pragma once

#include "Engine/ECS/core/Component.h"
#include "Engine/ECS/core/World.h"

namespace FrostFireEngine
{
  class ScriptSystem;

  class ScriptComponent : public Component
  {
  public:
    ~ScriptComponent() override = default;

    virtual void Awake()
    {
    }
    virtual void Start()
    {
    }
    virtual void Update(float deltaTime)
    {
    }
    virtual void OnCollisionEnter(EntityId other)
    {
    }
    virtual void OnCollisionStay(EntityId other)
    {
    }
    virtual void OnCollisionExit(EntityId other)
    {
    }
    virtual void OnTriggerEnter(EntityId other)
    {
    }
    virtual void OnTriggerStay(EntityId other)
    {
    }
    virtual void OnTriggerExit(EntityId other)
    {
    }
    virtual void OnEnable()
    {
    }
    virtual void OnDisable()
    {
    }

    void OnAttach() override;
    void OnDetach() override;

    void SetEnabled(bool value) noexcept override;

    [[nodiscard]] bool HasStarted() const
    {
      return started;
    }
    [[nodiscard]] bool HasAwoken() const
    {
      return awoken;
    }
    [[nodiscard]] Entity* GetEntity() const
    {
      return World::GetInstance().GetEntity(owner).get();
    }
    [[nodiscard]] EntityId GetOwnerId() const
    {
      return owner;
    }

  private:
    void RegisterWithScriptSystem();
    void UnregisterFromScriptSystem();

    ScriptSystem* scriptSystem = nullptr;
    bool started = false;
    bool awoken = false;
    friend class ScriptSystem;
  };
}
