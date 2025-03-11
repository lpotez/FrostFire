#pragma once
#include "Engine/Types.h"

namespace FrostFireEngine
{
  class Component
  {
  public:
    virtual ~Component() = default;

    virtual void OnAttach()
    {
    }
    virtual void OnDetach()
    {
    }

    constexpr EntityId GetOwner() const noexcept { return owner; }

    bool IsEnabled() const noexcept { return enabled; }
    virtual void SetEnabled(bool value) noexcept { enabled = value; }

  protected:
    EntityId owner{INVALID_ENTITY_ID};
    bool enabled = true; // par défaut activé

    friend class Entity;
    friend class ComponentManager;
  };
}
