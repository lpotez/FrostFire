#pragma once
#include "Engine/stdafx.h"
#include <vector>
#include <memory>
#include "PxSimulationEventCallback.h"
#include "Engine/ECS/core/World.h"

#include "Engine/Core/PhysicsResources.h"

namespace FrostFireEngine
{
  struct CollisionEvent {
    EntityId           entityA;
    EntityId           entityB;
    physx::PxPairFlags flags;
  };

  struct TriggerEvent {
    EntityId                entityA;
    EntityId                entityB;
    physx::PxPairFlag::Enum flags;
  };

  class PhysicsSystem;

  class CollisionCallback : public physx::PxSimulationEventCallback {
  public:
    CollisionCallback(PhysicsSystem& physicsSystem);
    void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
    void onContact(const physx::PxContactPairHeader& pairHeader,
                   const physx::PxContactPair*       pairs,
                   physx::PxU32                      nbPairs) override;
    void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;
    void onWake(physx::PxActor** actors, physx::PxU32 count) override;
    void onSleep(physx::PxActor** actors, physx::PxU32 count) override;
    void onAdvance(const physx::PxRigidBody* const* bodyBuffer,
                   const physx::PxTransform*        poseBuffer,
                   const physx::PxU32               count) override;

  private:
    PhysicsSystem& physicsSystem;
  };


  class PhysicsSystem : public System {
    friend class CollisionCallback;

  public:
    PhysicsSystem();
    ~PhysicsSystem() override;

    PhysicsSystem(const PhysicsSystem&) = delete;
    PhysicsSystem& operator=(const PhysicsSystem&) = delete;

    void Initialize() override;
    void Update(float deltaTime) override;
    void Cleanup() override;

    physx::PxPhysics* GetPhysics() const
    {
      return PhysicsResources::GetInstance().GetPhysics();
    }
    physx::PxScene* GetScene() const
    {
      return scene;
    }
    physx::PxMaterial* GetMaterial() const
    {
      return PhysicsResources::GetInstance().GetMaterial();
    }

    static PhysicsSystem& Get()
    {
      auto* system = World::GetInstance().GetSystem<PhysicsSystem>();
      if (!system) {
        throw std::runtime_error("PhysicsSystem not initialized");
      }
      return *system;
    }

  private:
    float                                    accumulatedTime = 0.0f;
    physx::PxScene*                          scene = nullptr;
    std::unique_ptr<class CollisionCallback> collisionCallback;
    std::vector<CollisionEvent>              collisionEvents;
    std::vector<TriggerEvent>                triggerEvents;

    static physx::PxFilterFlags CustomFilterShader(
      physx::PxFilterObjectAttributes attributes0,
      physx::PxFilterData             filterData0,
      physx::PxFilterObjectAttributes attributes1,
      physx::PxFilterData             filterData1,
      physx::PxPairFlags&             pairFlags,
      const void*                     constantBlock,
      physx::PxU32                    constantBlockSize
      );

    void AddCollisionEvent(const physx::PxContactPairHeader& pairHeader,
                           const physx::PxContactPair&       pair,
                           physx::PxPairFlag::Enum           flag);
    void ProcessCollisionEvents();
    void AddTriggerEvent(const physx::PxTriggerPair& pair, physx::PxPairFlag::Enum flag);
    void ProcessTriggerEvents();
  };
}
