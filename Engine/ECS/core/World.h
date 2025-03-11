#pragma once
#include <memory>
#include <span>
#include <string>
#include <vector>
#include <unordered_map>
#include <bitset>
#include <typeindex>
#include <stdexcept>
#include <bitset>
#include <unordered_map>
#include <typeindex>

#include "Engine/Types.h"
#include "ComponentManager.h"
#include "System.h"
#include "Engine/Scene/Octree.h"
#include "Entity.h"

namespace FrostFireEngine
{
  class World final {
    friend class System;

  public:
    World();
    ~World();

    template <typename... Components>
    [[nodiscard]] std::shared_ptr<Entity> CreateEntity()
    {
      if constexpr (sizeof...(Components) == 0) {
        return CreateEntityImpl();
      }
      else {
        auto entity = CreateEntityImpl();
        (entity->AddComponent<Components>(), ...);
        return entity;
      }
    }

    [[nodiscard]] std::shared_ptr<Entity> GetEntity(EntityId id) const noexcept;
    void                                  DestroyEntity(const std::shared_ptr<Entity>& entity);

    std::span<const std::shared_ptr<Entity>> GetEntitiesWithMask(ComponentMask requiredMask) const
    {
      // Cache local thread pour éviter des réallocations inutiles
      thread_local std::vector<std::shared_ptr<Entity>> cachedResults;
      thread_local ComponentMask                        lastQueryMask = 0;
      thread_local size_t                               lastVersion = 0;

      if (lastQueryMask == requiredMask && lastVersion == entityVersion) {
        return std::span{cachedResults};
      }

      cachedResults.clear();
      cachedResults.reserve(entities.size() / 4);

      for (const auto& entity : entities) {
        // Vérifie que l'entité possède au moins tous les composants du masque requis
        if ((entity->GetComponentMask() & requiredMask) == requiredMask) {
          cachedResults.push_back(entity);
        }
      }

      lastQueryMask = requiredMask;
      lastVersion = entityVersion;

      return std::span{cachedResults};
    }

    // On ajoute le filtrage des entités et composants désactivés ici
    template <typename... Components>
    [[nodiscard]] std::span<std::shared_ptr<Entity>> GetEntitiesWith() const
    {
      // Construit un masque incluant les types demandés et leurs dérivés
      ComponentMask requiredMask = 0;
      ((requiredMask |= ComponentManager::GetMaskForComponentAndDerived<Components>()), ...);
      // Récupère d'abord toutes les entités qui ont les composants demandés
      auto baseEntities = GetEntitiesWithMask(requiredMask);

      // Filtrage final
      thread_local std::vector<std::shared_ptr<Entity>> filteredResults;
      filteredResults.clear();
      filteredResults.reserve(baseEntities.size());

      for (const auto& entity : baseEntities) {
        if (!entity->IsEnabled()) {
          continue; // l'entité est désactivée, on l'ignore
        }

        // Vérifie chaque composant demandé
        bool allComponentsEnabled = true;
        ((allComponentsEnabled = allComponentsEnabled &&
            (entity->template GetComponent<Components>() &&
             entity->template GetComponent<Components>()->IsEnabled())), ...);

        if (allComponentsEnabled) {
          filteredResults.push_back(entity);
        }
      }

      return std::span{filteredResults};
    }

    template <typename... Components, typename Func>
    void ForEachComponent(Func&& func) const
    {
      auto entities = GetEntitiesWith<Components...>();
      for (const auto& entity : entities) {
        func(entity->template GetComponent<Components>()...);
      }
    }

    template <typename T, typename... Args>
    T& AddSystem(SystemPhase phase, Args&&... args)
    {
      static_assert(std::is_base_of_v<System, T>, "Type must inherit from System");
      if (auto* existing = GetSystem<T>()) {
        return *existing;
      }
      auto system = std::make_unique<T>(std::forward<Args>(args)...);
      if (!system) {
        throw std::runtime_error("Failed to create system instance");
      }
      T&           systemRef = *system;
      const size_t phaseIndex = static_cast<size_t>(phase);
      if (phaseIndex >= static_cast<size_t>(SystemPhase::Count)) {
        throw std::runtime_error("Invalid system phase: " + std::to_string(phaseIndex));
      }
      systems[typeid(T)] = std::move(system);
      orderedSystems[phaseIndex].push_back(&systemRef);
      return systemRef;
    }

    template <typename T>
    [[nodiscard]] T* GetSystem() const noexcept
    {
      auto it = systems.find(typeid(T));
      if (it != systems.end()) {
        return static_cast<T*>(it->second.get());
      }
      return nullptr;
    }

    void                                                   Update(float deltaTime);
    void                                                   Init();
    void                                                   InitializeSystems() const;
    void                                                   Clear();
    [[nodiscard]] std::span<const std::shared_ptr<Entity>> GetEntities() const noexcept;

    static World& GetInstance();

    void InsertOctreeEntity(EntityId id);
    void UpdateOctreeEntity(EntityId id);
    void RemoveOctreeEntity(EntityId id) const;

    Octree& GetOctree() { return sceneOctree; }
    void BuildOctree();
    bool IsOctreeBuilt() const { return octreeBuilt; }
  private:
    static constexpr size_t                                      DESTROY_BUFFER_SIZE = 64;
    std::shared_ptr<Entity>                                      CreateEntityImpl();
    EntityId                                                     FindNextAvailableId() const;
    void                                                         ProcessDestroyBuffer();
    std::vector<std::shared_ptr<Entity>>                         entities;
    std::vector<std::shared_ptr<Entity>>                         destroyBuffer;
    std::vector<ComponentMask>                                   entitySignatures;
    std::bitset<MAX_ENTITIES>                                    availableIds;
    EntityId                                                     nextEntityId = 0;
    size_t                                                       entityVersion = 0;
    mutable std::unordered_map<EntityId, std::weak_ptr<Entity>>  entityCache;
    std::unordered_map<std::type_index, std::unique_ptr<System>> systems;
    std::vector<std::vector<System*>>                            orderedSystems;
    ComponentManager                                             componentManager;

    Octree sceneOctree;
    bool octreeBuilt = false;
  };
}
