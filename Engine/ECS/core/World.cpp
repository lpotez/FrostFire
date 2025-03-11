#include "Engine/ECS/core/World.h"
#include "Engine/SceneManager.h"
#include "Engine/ECS/components/mesh/MeshComponent.h"
#include "Engine/ECS/components/rendering/PBRRenderer.h"
#include "Engine/ECS/components/transform/TransformComponent.h"
#include <algorithm>
#include <stdexcept>
#include <cfloat>
#include <functional>

namespace FrostFireEngine
{
  #undef min
  #undef max

  World::World()
    : sceneOctree(AABB{XMFLOAT3(-300, -300, -300), XMFLOAT3(300, 300, 300)})
  {
    Init();
  }

  World::~World()
  {
    Clear();
  }

  World& World::GetInstance()
  {
    auto activeScene = SceneManager::GetInstance().GetActiveScene();
    if (!activeScene) {
      throw std::runtime_error("No active scene");
    }
    return activeScene->GetWorld();
  }

  void World::InsertOctreeEntity(EntityId id)
  {
    auto entity = GetEntity(id);
    if (!entity) return;

    auto meshComp = entity->GetComponent<MeshComponent>();
    auto transform = entity->GetComponent<TransformComponent>();
    auto renderer = entity->GetComponent<BaseRendererComponent>();

    if (!meshComp || !transform || !renderer) return;

    auto mesh = meshComp->GetMesh();
    if (!mesh) return;

    // On récupère la boîte locale du mesh
    const auto& localBox = mesh->GetBounds().box;

    // On calcule les 8 coins de la boîte locale
    XMFLOAT3 corners[8] = {
      {localBox.min.x, localBox.min.y, localBox.min.z},
      {localBox.min.x, localBox.min.y, localBox.max.z},
      {localBox.min.x, localBox.max.y, localBox.min.z},
      {localBox.min.x, localBox.max.y, localBox.max.z},
      {localBox.max.x, localBox.min.y, localBox.min.z},
      {localBox.max.x, localBox.min.y, localBox.max.z},
      {localBox.max.x, localBox.max.y, localBox.min.z},
      {localBox.max.x, localBox.max.y, localBox.max.z},
    };

    XMMATRIX worldMatrix = transform->GetWorldMatrix();
    XMFLOAT3 worldMin = {FLT_MAX, FLT_MAX, FLT_MAX};
    XMFLOAT3 worldMax = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

    for (auto& c : corners) {
      XMVECTOR v = XMLoadFloat3(&c);
      v = XMVector3Transform(v, worldMatrix);
      XMFLOAT3 wc;
      XMStoreFloat3(&wc, v);

      worldMin.x = std::min(worldMin.x, wc.x);
      worldMin.y = std::min(worldMin.y, wc.y);
      worldMin.z = std::min(worldMin.z, wc.z);

      worldMax.x = std::max(worldMax.x, wc.x);
      worldMax.y = std::max(worldMax.y, wc.y);
      worldMax.z = std::max(worldMax.z, wc.z);
    }

    AABB worldAABB{worldMin, worldMax};
    sceneOctree.InsertRenderer(renderer, worldAABB);
  }

  void World::UpdateOctreeEntity(EntityId id)
  {
    auto entity = GetEntity(id);
    if (!entity) return;

    auto renderer = entity->GetComponent<BaseRendererComponent>();
    if (!renderer) return;

    auto meshComp = entity->GetComponent<MeshComponent>();
    auto transform = entity->GetComponent<TransformComponent>();
    if (!meshComp || !transform) return;

    auto mesh = meshComp->GetMesh();
    if (!mesh) return;

    const auto& localBox = mesh->GetBounds().box;

    XMFLOAT3 corners[8] = {
      {localBox.min.x, localBox.min.y, localBox.min.z},
      {localBox.min.x, localBox.min.y, localBox.max.z},
      {localBox.min.x, localBox.max.y, localBox.min.z},
      {localBox.min.x, localBox.max.y, localBox.max.z},
      {localBox.max.x, localBox.min.y, localBox.min.z},
      {localBox.max.x, localBox.min.y, localBox.max.z},
      {localBox.max.x, localBox.max.y, localBox.min.z},
      {localBox.max.x, localBox.max.y, localBox.max.z},
    };

    XMMATRIX worldMatrix = transform->GetWorldMatrix();
    XMFLOAT3 worldMin = {FLT_MAX, FLT_MAX, FLT_MAX};
    XMFLOAT3 worldMax = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

    for (auto& c : corners) {
      XMVECTOR v = XMLoadFloat3(&c);
      v = XMVector3Transform(v, worldMatrix);
      XMFLOAT3 wc;
      XMStoreFloat3(&wc, v);

      worldMin.x = std::min(worldMin.x, wc.x);
      worldMin.y = std::min(worldMin.y, wc.y);
      worldMin.z = std::min(worldMin.z, wc.z);

      worldMax.x = std::max(worldMax.x, wc.x);
      worldMax.y = std::max(worldMax.y, wc.y);
      worldMax.z = std::max(worldMax.z, wc.z);
    }

    AABB worldAABB{worldMin, worldMax};
    sceneOctree.UpdateRenderer(renderer, worldAABB);
  }

  void World::RemoveOctreeEntity(EntityId id) const
  {
    auto entity = GetEntity(id);
    if (!entity) return;

    auto renderer = entity->GetComponent<BaseRendererComponent>();
    if (!renderer) return;

    sceneOctree.RemoveRenderer(renderer);
  }
  void World::BuildOctree()
  {
    // Effacer l'octree
    sceneOctree.Clear();

    auto entities = GetEntitiesWith<PBRRenderer, TransformComponent, MeshComponent>();

    for (auto& entity : entities) {
      auto renderer = entity->GetComponent<BaseRendererComponent>();
      auto transform = entity->GetComponent<TransformComponent>();
      auto meshComp = entity->GetComponent<MeshComponent>();
      if (!renderer || !transform || !meshComp) continue;

      const auto& localBox = meshComp->GetMesh()->GetBounds().box;
      XMFLOAT3    corners[8] = {
        {localBox.min.x, localBox.min.y, localBox.min.z},
        {localBox.min.x, localBox.min.y, localBox.max.z},
        {localBox.min.x, localBox.max.y, localBox.min.z},
        {localBox.min.x, localBox.max.y, localBox.max.z},
        {localBox.max.x, localBox.min.y, localBox.min.z},
        {localBox.max.x, localBox.min.y, localBox.max.z},
        {localBox.max.x, localBox.max.y, localBox.min.z},
        {localBox.max.x, localBox.max.y, localBox.max.z},
      };

      XMMATRIX worldMatrix = transform->GetWorldMatrix();
      XMFLOAT3 worldMin = {FLT_MAX, FLT_MAX, FLT_MAX};
      XMFLOAT3 worldMax = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

      for (auto& c : corners) {
        XMVECTOR v = XMLoadFloat3(&c);
        v = XMVector3Transform(v, worldMatrix);
        XMFLOAT3 wc;
        XMStoreFloat3(&wc, v);

        worldMin.x = std::min(worldMin.x, wc.x);
        worldMin.y = std::min(worldMin.y, wc.y);
        worldMin.z = std::min(worldMin.z, wc.z);

        worldMax.x = std::max(worldMax.x, wc.x);
        worldMax.y = std::max(worldMax.y, wc.y);
        worldMax.z = std::max(worldMax.z, wc.z);
      }

      AABB worldAABB{worldMin, worldMax};
      sceneOctree.InsertRenderer(renderer, worldAABB);
    }

    octreeBuilt = true;
  }

  std::shared_ptr<Entity> World::GetEntity(const EntityId id) const noexcept
  {
    if (auto it = entityCache.find(id); it != entityCache.end()) {
      if (auto entity = it->second.lock()) {
        return entity;
      }
      entityCache.erase(it);
    }
    auto it = std::find_if(entities.begin(), entities.end(),
                           [id](const auto& entity)
                           {
                             return entity->GetId() == id;
                           });
    if (it != entities.end()) {
      entityCache[id] = *it;
      return *it;
    }
    return nullptr;
  }

  void World::DestroyEntity(const std::shared_ptr<Entity>& entity)
  {
    if (!entity) return;
    destroyBuffer.push_back(entity);
    if (destroyBuffer.size() >= DESTROY_BUFFER_SIZE) {
      ProcessDestroyBuffer();
    }
  }

  std::shared_ptr<Entity> World::CreateEntityImpl()
  {
    EntityId id;
    if (nextEntityId < MAX_ENTITIES && !availableIds.test(nextEntityId)) {
      id = nextEntityId++;
    }
    else {
      id = FindNextAvailableId();
    }
    availableIds.set(id);
    auto entity = Entity::EntityCreator::Create(id, componentManager);
    entities.push_back(entity);
    if (id >= entitySignatures.size()) {
      entitySignatures.resize(id + 1, 0);
    }
    entitySignatures[id] = 0;
    entityCache[id] = entity;
    entityVersion++;
    return entity;
  }

  EntityId World::FindNextAvailableId() const
  {
    for (EntityId i = 0; i < MAX_ENTITIES; ++i) {
      if (!availableIds.test(i)) {
        return i;
      }
    }
    throw std::runtime_error("No available entity IDs");
  }

  void World::ProcessDestroyBuffer()
  {
    if (destroyBuffer.empty()) return;
    for (const auto& entity : destroyBuffer) {
      const EntityId      id = entity->GetId();
      const ComponentMask mask = entity->GetComponentMask();
      for (size_t i = 0; i < MAX_COMPONENTS; ++i) {
        if (mask & (1u << i)) {
          componentManager.DestroyComponentByType(id, i);
        }
      }
      if (id < entitySignatures.size()) {
        entitySignatures[id] = 0;
      }
      entityCache.erase(id);
      availableIds.reset(id);
    }
    std::erase_if(entities,
                  [&](const std::shared_ptr<Entity>& e)
                  {
                    return std::any_of(destroyBuffer.begin(), destroyBuffer.end(),
                                       [&](const std::shared_ptr<Entity>& d)
                                       {
                                         return d->GetId() == e->GetId();
                                       });
                  });
    destroyBuffer.clear();
    entityVersion++;
  }

  void World::Update(const float deltaTime)
  {
    ProcessDestroyBuffer();
    for (size_t phase = 0; phase < static_cast<size_t>(SystemPhase::Count); ++phase) {
      for (auto* system : orderedSystems[phase]) {
        if (system) system->Update(deltaTime);
      }
    }
  }

  void World::Init()
  {
    static constexpr size_t INITIAL_CAPACITY = MAX_ENTITIES / 4;
    entities.reserve(INITIAL_CAPACITY);
    entitySignatures.reserve(INITIAL_CAPACITY);
    destroyBuffer.reserve(DESTROY_BUFFER_SIZE);
    orderedSystems.resize(static_cast<size_t>(SystemPhase::Count));
    availableIds.reset();
    nextEntityId = 0;
    entityVersion = 0;
  }

  void World::InitializeSystems() const
  {
    for (size_t phase = 0; phase < static_cast<size_t>(SystemPhase::Count); ++phase) {
      for (auto* system : orderedSystems[phase]) {
        if (system) {
          system->Initialize();
        }
      }
    }
  }

  void World::Clear()
  {
    octreeBuilt = false;
    sceneOctree.Clear();

    ProcessDestroyBuffer();

    // Nettoyage des systèmes
    for (auto& phase : orderedSystems) {
      for (auto* system : phase) {
        if (system) system->Cleanup();
      }
      phase.clear();
    }

    systems.clear();
    entities.clear();
    entitySignatures.clear();
    entityCache.clear();
    availableIds.reset();
    nextEntityId = 0;
    entityVersion++;
  }


  std::span<const std::shared_ptr<Entity>> World::GetEntities() const noexcept
  {
    return entities;
  }
}
