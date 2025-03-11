#pragma once
#include <array>
#include <bitset>
#include <cassert>
#include <memory>
#include <span>
#include <limits>

#include "Component.h"

#undef max
#undef min

namespace FrostFireEngine
{
  class IComponentPool {
  public:
    virtual             ~IComponentPool() = default;
    virtual void        Remove(EntityId entityId) noexcept = 0;
    virtual void        Clear() noexcept = 0;
    virtual void*       GetVoidPtr(EntityId entityId) noexcept = 0;
    virtual const void* GetVoidPtr(EntityId entityId) const noexcept = 0;
  };

  template <typename T>
  class ComponentPool : public IComponentPool {
  public:
    using PtrType = std::unique_ptr<T>;
    static constexpr uint32_t INVALID_INDEX = std::numeric_limits<uint32_t>::max();

    ComponentPool()
    {
      for (auto& idx : indices) {
        idx = INVALID_INDEX;
      }
    }

    ~ComponentPool() override
    {
      Clear();
    }

    template <typename U = T, typename... Args>
    U& Add(EntityId entityId, Args&&... args)
    {
      assert(entityId < MAX_ENTITIES);
      assert(size < MAX_ENTITIES);
      const size_t index = size++;
      indices[entityId] = static_cast<uint32_t>(index);
      activeComponents.set(index);
      data[index] = std::make_unique<U>(std::forward<Args>(args)...);
      entityIds[index] = entityId;
      version++;
      return *static_cast<U*>(data[index].get());
    }

    void Remove(EntityId entityId) noexcept override
    {
      if (entityId >= MAX_ENTITIES) return;
      const auto index = indices[entityId];
      if (index == INVALID_INDEX) return;

      if (size == 0) {
        return;
      }

      const size_t lastIndex = size - 1;
      if (index != lastIndex) {
        data[index] = std::move(data[lastIndex]);
        EntityId lastEntityId = entityIds[lastIndex];
        entityIds[index] = lastEntityId;
        assert(lastEntityId < MAX_ENTITIES);
        indices[lastEntityId] = static_cast<uint32_t>(index);
      }
      else {
        data[index].reset();
      }
      indices[entityId] = INVALID_INDEX;
      activeComponents.reset(lastIndex);
      --size;
      version++;
    }

    void* GetVoidPtr(EntityId entityId) noexcept override
    {
      return const_cast<void*>(static_cast<const ComponentPool*>(this)->GetVoidPtr(entityId));
    }

    const void* GetVoidPtr(EntityId entityId) const noexcept override
    {
      const T* ptr = Get(entityId);
      return static_cast<const void*>(ptr);
    }

    T* Get(EntityId entityId) noexcept
    {
      if (entityId >= MAX_ENTITIES) return nullptr;
      const auto index = indices[entityId];
      return index != INVALID_INDEX ? data[index].get() : nullptr;
    }

    const T* Get(EntityId entityId) const noexcept
    {
      if (entityId >= MAX_ENTITIES) return nullptr;
      const auto index = indices[entityId];
      return index != INVALID_INDEX ? data[index].get() : nullptr;
    }

    void Clear() noexcept override
    {
      for (size_t i = 0; i < size; ++i) {
        if (activeComponents.test(i)) {
          data[i].reset();
        }
      }
      size = 0;
      for (auto& idx : indices) {
        idx = INVALID_INDEX;
      }
      activeComponents.reset();
      version++;
    }

    std::span<T*> GetAll() noexcept
    {
      UpdateCacheIfNeeded();
      return std::span<T*>(cachedPointers.data(), cachedPointers.size());
    }

    std::span<const T*> GetAll() const noexcept
    {
      UpdateCacheIfNeeded();
      return std::span<const T*>(cachedPointers.data(), cachedPointers.size());
    }

    size_t Size() const noexcept
    {
      return size;
    }

    bool Empty() const noexcept
    {
      return size == 0;
    }

  private:
    std::array<PtrType, MAX_ENTITIES>  data;
    std::array<EntityId, MAX_ENTITIES> entityIds{};
    std::array<uint32_t, MAX_ENTITIES> indices;
    std::bitset<MAX_ENTITIES>          activeComponents;
    size_t                             size = 0;
    mutable size_t                     cacheVersion = 0;
    size_t                             version = 0;
    mutable std::vector<T*>            cachedPointers;

    void UpdateCacheIfNeeded() const
    {
      if (cacheVersion == version) return;
      auto nonConstThis = const_cast<ComponentPool<T>*>(this);
      nonConstThis->cachedPointers.clear();
      nonConstThis->cachedPointers.reserve(size);
      for (size_t i = 0; i < size; ++i) {
        if (activeComponents.test(i) && data[i]) {
          nonConstThis->cachedPointers.push_back(data[i].get());
        }
      }
      cacheVersion = version;
    }
  };
}
