#include "Engine/stdafx.h"
#include "PhysicsSystem.h"
#include "Engine/ECS/core/World.h"
#include "ScriptSystem.h"
#include <vector>
#include <iostream>

#include "PauseManagerSystem.h"
#include "Engine/ECS/components/physics/RigidBodyComponent.h"
#include "Engine/ECS/components/transform/TransformComponent.h"
#include "PxScene.h"
#include "PxFiltering.h"
#include "Engine/Core/PhysicsResources.h"

namespace FrostFireEngine
{
  PhysicsSystem::PhysicsSystem()
  {
    // Le constructeur n’a plus besoin de gérer les ressources globales
    // PhysicsResources::GetInstance() s’en occupe lors de la première utilisation.
  }

  PhysicsSystem::~PhysicsSystem()
  {
    // Plus de gestion des ressources globales ici.
  }

  void PhysicsSystem::Initialize()
  {
    physx::PxPhysics*              physics = PhysicsResources::GetInstance().GetPhysics();
    physx::PxDefaultCpuDispatcher* dispatcher = PhysicsResources::GetInstance().GetDispatcher();

    physx::PxSceneDesc sceneDesc(physics->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
    sceneDesc.filterShader = CustomFilterShader;
    sceneDesc.cpuDispatcher = dispatcher;
    scene = physics->createScene(sceneDesc);
    if (!scene) {
      throw std::runtime_error("Failed to create PxScene");
    }
    collisionCallback = std::make_unique<CollisionCallback>(*this);
    scene->setSimulationEventCallback(collisionCallback.get());
  }

  void PhysicsSystem::Update(float deltaTime)
  {
    if (PauseManagerSystem::Get().IsPaused()) {
      return;
    }

    if (!scene) return;

    accumulatedTime += deltaTime;
    constexpr float fixedTimeStep = 1.0f / 60.0f;

    while (accumulatedTime >= fixedTimeStep) {
      scene->simulate(fixedTimeStep);
      if (scene->fetchResults(true)) {
        ProcessCollisionEvents();
        ProcessTriggerEvents();
        scene->flushSimulation();
      }
      accumulatedTime -= fixedTimeStep;
    }

    for (const auto entities = GetEntitiesWith<RigidBodyComponent, TransformComponent>(); const auto
         &          entity : entities) {
      const auto* rigidBody = entity->GetComponent<RigidBodyComponent>();
      auto*       transform = entity->GetComponent<TransformComponent>();
      if (rigidBody->GetType() == RigidBodyComponent::Type::Dynamic) {
        const physx::PxTransform pxTransform = rigidBody->GetActor()->getGlobalPose();
        transform->SetWorldPosition({pxTransform.p.x, pxTransform.p.y, pxTransform.p.z});
        transform->SetWorldRotation({
          pxTransform.q.x, pxTransform.q.y, pxTransform.q.z, pxTransform.q.w});
      }
    }
  }

  void PhysicsSystem::Cleanup()
  {
    collisionCallback.reset();
  }

  physx::PxFilterFlags PhysicsSystem::CustomFilterShader(
    physx::PxFilterObjectAttributes attributes0,
    physx::PxFilterData             filterData0,
    physx::PxFilterObjectAttributes attributes1,
    physx::PxFilterData             filterData1,
    physx::PxPairFlags&             pairFlags,
    const void*                     constantBlock,
    physx::PxU32                    constantBlockSize)
  {
    PX_UNUSED(filterData0);
    PX_UNUSED(filterData1);
    PX_UNUSED(constantBlock);
    PX_UNUSED(constantBlockSize);

    if (physx::PxFilterObjectIsTrigger(attributes0) ||
      physx::PxFilterObjectIsTrigger(attributes1)) {
      pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
      return physx::PxFilterFlag::eDEFAULT;
    }

    pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT |
    physx::PxPairFlag::eNOTIFY_TOUCH_FOUND |
    physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS |
    physx::PxPairFlag::eNOTIFY_TOUCH_LOST;

    return physx::PxFilterFlag::eDEFAULT;
  }

  void PhysicsSystem::AddCollisionEvent(const physx::PxContactPairHeader& pairHeader,
                                        const physx::PxContactPair&       pair,
                                        physx::PxPairFlag::Enum           flag)
  {
    EntityId entityA = static_cast<Entity*>(pairHeader.actors[0]->userData)->GetId();
    EntityId entityB = static_cast<Entity*>(pairHeader.actors[1]->userData)->GetId();
    collisionEvents.emplace_back(CollisionEvent{entityA, entityB, flag});
  }

  void PhysicsSystem::ProcessCollisionEvents()
  {
    ScriptSystem* scriptSystem = World::GetInstance().GetSystem<ScriptSystem>();
    if (!scriptSystem) {
      std::cerr << "ScriptSystem not found!\n";
      return;
    }
    for (const auto& event : collisionEvents) {
      if (event.flags & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) {
        scriptSystem->OnCollisionEnter(event.entityA, event.entityB);
      }
      if (event.flags & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS) {
        scriptSystem->OnCollisionStay(event.entityA, event.entityB);
      }
      if (event.flags & physx::PxPairFlag::eNOTIFY_TOUCH_LOST) {
        scriptSystem->OnCollisionExit(event.entityA, event.entityB);
      }
    }
    collisionEvents.clear();
  }

  void PhysicsSystem::AddTriggerEvent(const physx::PxTriggerPair& pair,
                                      physx::PxPairFlag::Enum     flag)
  {
    EntityId triggerEntity = static_cast<Entity*>(pair.triggerActor->userData)->GetId();
    EntityId otherEntity = static_cast<Entity*>(pair.otherActor->userData)->GetId();
    triggerEvents.emplace_back(TriggerEvent{triggerEntity, otherEntity, flag});
  }

  void PhysicsSystem::ProcessTriggerEvents()
  {
    ScriptSystem* scriptSystem = World::GetInstance().GetSystem<ScriptSystem>();
    if (!scriptSystem) return;
    for (const auto& event : triggerEvents) {
      if (event.flags & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) {
        scriptSystem->OnTriggerEnter(event.entityA, event.entityB);
      }
      if (event.flags & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS) {
        scriptSystem->OnTriggerStay(event.entityA, event.entityB);
      }
      if (event.flags & physx::PxPairFlag::eNOTIFY_TOUCH_LOST) {
        scriptSystem->OnTriggerExit(event.entityA, event.entityB);
      }
    }
    triggerEvents.clear();
  }
  CollisionCallback::CollisionCallback(PhysicsSystem& physicsSystem) : physicsSystem(
    physicsSystem)
  {
  }

  void CollisionCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
  {
    for (physx::PxU32 i = 0; i < count; i++) {
      const physx::PxTriggerPair& pair = pairs[i];
      if (pair.flags & physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER ||
        pair.flags & physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER)
        continue;
      if (pair.status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) {
        physicsSystem.AddTriggerEvent(pair, physx::PxPairFlag::eNOTIFY_TOUCH_FOUND);
      }
      else if (pair.status & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS) {
        physicsSystem.AddTriggerEvent(pair, physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS);
      }
      else if (pair.status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST) {
        physicsSystem.AddTriggerEvent(pair, physx::PxPairFlag::eNOTIFY_TOUCH_LOST);
      }
    }
  }

  void CollisionCallback::onContact(const physx::PxContactPairHeader& pairHeader,
                                    const physx::PxContactPair*       pairs,
                                    physx::PxU32                      nbPairs)
  {
    for (physx::PxU32 i = 0; i < nbPairs; ++i) {
      const physx::PxContactPair& pair = pairs[i];
      if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) {
        physicsSystem.AddCollisionEvent(pairHeader, pair, physx::PxPairFlag::eNOTIFY_TOUCH_FOUND);
      }
      if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS) {
        physicsSystem.AddCollisionEvent(pairHeader, pair,
                                        physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS);
      }
      if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST) {
        physicsSystem.AddCollisionEvent(pairHeader, pair, physx::PxPairFlag::eNOTIFY_TOUCH_LOST);
      }
    }
  }

  void CollisionCallback::onConstraintBreak(physx::PxConstraintInfo* constraints,
                                            physx::PxU32             count)
  {
    PX_UNUSED(constraints);
    PX_UNUSED(count);
  }

  void CollisionCallback::onWake(physx::PxActor** actors, physx::PxU32 count)
  {
    PX_UNUSED(actors);
    PX_UNUSED(count);
  }

  void CollisionCallback::onSleep(physx::PxActor** actors, physx::PxU32 count)
  {
    PX_UNUSED(actors);
    PX_UNUSED(count);
  }

  void CollisionCallback::onAdvance(const physx::PxRigidBody* const* bodyBuffer,
                                    const physx::PxTransform*        poseBuffer,
                                    const physx::PxU32               count)
  {
    PX_UNUSED(bodyBuffer);
    PX_UNUSED(poseBuffer);
    PX_UNUSED(count);
  }
}
