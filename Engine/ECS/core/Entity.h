#pragma once
#include "ComponentManager.h"
#include <memory>
#include <stdexcept>

namespace FrostFireEngine
{
  class Entity : public std::enable_shared_from_this<Entity> {
    friend class World;

  public:
    template <typename T, typename... Args>
    T& AddComponent(Args&&... args)
    {
      auto componentType = ComponentManager::GetComponentType<T>();
      if (componentType >= MAX_COMPONENTS) {
        throw std::runtime_error("Component type exceeds MAX_COMPONENTS");
      }

      componentMask |= (1u << componentType);
      return componentManager.AddComponent<T>(id, std::forward<Args>(args)...);
    }

    template <typename T>
    T* GetComponent()
    {
      return componentManager.GetComponent<T>(id);
    }

    template <typename T>
    const T* GetComponent() const
    {
      return componentManager.GetComponent<T>(id);
    }

    template <typename T>
    bool HasComponent() const
    {
      return (componentManager.GetComponent<T>(id) != nullptr);
    }

    template <typename T>
    void RemoveComponent()
    {
      if (auto* comp = componentManager.GetComponent<T>(id)) {
        if constexpr (std::is_base_of_v<Component, T>) {
          comp->OnDetach();
        }

        componentManager.RemoveComponent<T>(id);

        auto componentType = ComponentManager::GetComponentType<T>();
        if (componentType < MAX_COMPONENTS) {
          componentMask &= ~(1u << componentType);
        }
      }
    }

    EntityId GetId() const noexcept
    {
      return id;
    }
    ComponentMask GetComponentMask() const noexcept
    {
      return componentMask;
    }

    bool IsEnabled() const noexcept
    {
      return enabled;
    }

    void SetEnabled(bool value) noexcept
    {
      enabled = value;
    }

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

  private:
    class EntityCreator {
      friend class World;
      static std::shared_ptr<Entity> Create(EntityId id, ComponentManager& componentManager)
      {
        if (id >= MAX_ENTITIES) {
          throw std::runtime_error("Entity ID exceeds MAX_ENTITIES");
        }
        return std::shared_ptr<Entity>(new Entity(id, componentManager));
      }
    };

    friend class EntityCreator;

    explicit Entity(EntityId entityId, ComponentManager& componentManager)
      : id(entityId)
        , componentMask(0)
        , componentManager(componentManager)
        , enabled(true)
    {
    }

    const EntityId    id;
    ComponentMask     componentMask;
    ComponentManager& componentManager;
    bool              enabled;
  };
}
