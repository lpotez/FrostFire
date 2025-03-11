#pragma once
#include <array>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "ComponentPool.h"

namespace FrostFireEngine
{
  inline std::vector<std::optional<std::type_index>>              g_componentTypes;
  inline std::unordered_map<std::type_index, std::vector<size_t>> g_inheritanceMap;

  class ComponentManager {
  public:
    ComponentManager() = default;
    ~ComponentManager() = default;

    template <typename T>
    static size_t GetComponentType() noexcept
    {
      static const size_t typeID = GetNextTypeId();
      static bool         initialized = []()
      {
        g_componentTypes.resize(std::max(g_componentTypes.size(), typeID + 1));
        g_componentTypes[typeID] = std::type_index(typeid(T));
        return true;
      }();
      (void)initialized;
      return typeID;
    }

    template <typename Derived, typename Base>
    static void RegisterInheritance()
    {
      static_assert(std::is_base_of_v<Base, Derived>, "Derived n'hérite pas de Base");
      size_t derivedID = GetComponentType<Derived>();
      size_t baseID = GetComponentType<Base>();

      auto& derivedList = g_inheritanceMap[std::type_index(typeid(Base))];

      if (std::find(derivedList.begin(), derivedList.end(), derivedID) == derivedList.end()) {
        derivedList.push_back(derivedID);
      }
      if (std::find(derivedList.begin(), derivedList.end(), baseID) == derivedList.end()) {
        derivedList.push_back(baseID);
      }
    }

    void DestroyComponentByType(EntityId entityId, size_t componentType) noexcept
    {
      if (componentType < MAX_COMPONENTS && destroyers[componentType]) {
        destroyers[componentType](this, entityId);
      }
    }

    template <typename T, typename... Args>
    T& AddComponent(EntityId entityId, Args&&... args)
    {
      auto& pool = GetComponentPool<T>();
      auto& component = pool.template Add<T>(entityId, std::forward<Args>(args)...);
      if constexpr (std::is_base_of_v<Component, T>) {
        component.owner = entityId;
        component.OnAttach();
      }
      return component;
    }

    template <typename T>
    T* GetComponent(EntityId entityId)
    {
      auto it = g_inheritanceMap.find(std::type_index(typeid(T)));
      if (it == g_inheritanceMap.end()) {
        return GetComponentPool<T>().Get(entityId);
      }

      for (auto derivedID : it->second) {
        if (derivedID < componentPools.size() && componentPools[derivedID]) {
          void* ptr = componentPools[derivedID]->GetVoidPtr(entityId);
          if (ptr != nullptr) {
            return static_cast<T*>(ptr);
          }
        }
      }
      return nullptr;
    }

    template <typename T>
    const T* GetComponent(EntityId entityId) const
    {
      auto it = g_inheritanceMap.find(std::type_index(typeid(T)));
      if (it == g_inheritanceMap.end()) {
        return GetComponentPool<T>().Get(entityId);
      }

      for (auto derivedID : it->second) {
        if (derivedID < componentPools.size() && componentPools[derivedID]) {
          const void* ptr = componentPools[derivedID]->GetVoidPtr(entityId);
          if (ptr != nullptr) {
            return static_cast<const T*>(ptr);
          }
        }
      }
      return nullptr;
    }

    template <typename T>
    void RemoveComponent(EntityId entityId)
    {
      auto it = g_inheritanceMap.find(std::type_index(typeid(T)));
      if (it == g_inheritanceMap.end()) {
        if (auto* comp = GetComponent<T>(entityId)) {
          if constexpr (std::is_base_of_v<Component, T>) {
            comp->OnDetach();
          }
          GetComponentPool<T>().Remove(entityId);
        }
      }
      else {
        for (auto derivedID : it->second) {
          if (derivedID < componentPools.size() && componentPools[derivedID]) {
            void* ptr = componentPools[derivedID]->GetVoidPtr(entityId);
            if (ptr != nullptr) {
              DestroyComponentByType(entityId, derivedID);
              return;
            }
          }
        }
      }
    }

    template <typename T>
    std::span<T*> GetAllComponents()
    {
      return GetComponentPool<T>().GetAll();
    }

    template <typename T>
    std::span<const T*> GetAllComponents() const
    {
      return GetComponentPool<T>().GetAll();
    }

    void ResetAll() noexcept
    {
      for (auto& pool : componentPools) {
        if (pool) {
          pool->Clear();
        }
      }
    }

    template <typename T>
    static ComponentMask GetMaskForComponentAndDerived()
    {
      ComponentMask mask = 0;
      size_t        baseID = GetComponentType<T>();
      mask |= (1u << baseID);

      auto it = g_inheritanceMap.find(std::type_index(typeid(T)));
      if (it != g_inheritanceMap.end()) {
        for (auto derivedID : it->second) {
          mask |= (1u << derivedID);
        }
      }
      return mask;
    }

    ComponentManager(const ComponentManager&) = delete;
    ComponentManager& operator=(const ComponentManager&) = delete;
    ComponentManager(ComponentManager&&) = delete;
    ComponentManager& operator=(ComponentManager&&) = delete;

  private:
    mutable std::array<std::unique_ptr<IComponentPool>, MAX_COMPONENTS>       componentPools{};
    std::array<void(*)(ComponentManager*, EntityId) noexcept, MAX_COMPONENTS> destroyers{};

    template <typename T>
    ComponentPool<T>& GetComponentPool()
    {
      size_t type = GetComponentType<T>();
      if (!componentPools[type]) {
        componentPools[type] = std::make_unique<ComponentPool<T>>();
        destroyers[type] = +[](ComponentManager* self, EntityId id) noexcept
        {
          if (auto* comp = self->GetComponent<T>(id)) {
            if constexpr (std::is_base_of_v<Component, T>) {
              comp->OnDetach();
            }
            self->GetComponentPool<T>().Remove(id);
          }
        };
      }
      return static_cast<ComponentPool<T>&>(*componentPools[type]);
    }

    template <typename T>
    const ComponentPool<T>& GetComponentPool() const
    {
      size_t type = GetComponentType<T>();
      if (!componentPools[type]) {
        const_cast<ComponentManager*>(this)->componentPools[type] = std::make_unique<ComponentPool<
          T>>();
        const_cast<ComponentManager*>(this)->destroyers[type] = +[](
          ComponentManager* self,
          EntityId          id) noexcept
        {
          if (auto* comp = self->GetComponent<T>(id)) {
            if constexpr (std::is_base_of_v<Component, T>) {
              comp->OnDetach();
            }
            self->GetComponentPool<T>().Remove(id);
          }
        };
      }
      return static_cast<const ComponentPool<T>&>(*componentPools[type]);
    }

    static size_t GetNextTypeId()
    {
      static size_t nextTypeId = 0;
      return nextTypeId++;
    }
  };

  #define REGISTER_INHERITANCE(Derived, Base) \
  namespace { \
  const bool s_##Derived##_##Base##_registered = [](){ \
  FrostFireEngine::ComponentManager::RegisterInheritance<Derived, Base>(); \
  return true; \
  }(); \
  }
}
