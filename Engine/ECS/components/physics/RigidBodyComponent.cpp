#include "Engine/stdafx.h"
#include "RigidBodyComponent.h"
#include "ColliderComponent.h"
#include <PxPhysicsAPI.h>

#include "Engine/ECS/systems/PhysicsSystem.h"
#include "Engine/ECS/components/transform/TransformComponent.h"

namespace FrostFireEngine
{
  RigidBodyComponent::RigidBodyComponent(Type type, bool isTrigger)
    : bodyType(type)
      , isTrigger(isTrigger)
  {
  }

  RigidBodyComponent::~RigidBodyComponent()
  {
    if (actor) {
      if (auto* scene = actor->getScene()) {
        scene->removeActor(*actor, true);
      }

      const physx::PxU32 numShapes = actor->getNbShapes();
      if (numShapes > 0) {
        std::vector<physx::PxShape*> shapes(numShapes);
        actor->getShapes(shapes.data(), numShapes);

        for (auto* shape : shapes) {
          actor->detachShape(*shape);
        }
      }

      actor->userData = nullptr;
      actor->release();
      actor = nullptr;
    }
  }
  void RigidBodyComponent::UpdateMassProperties() const
  {
    if (bodyType == Type::Dynamic && actor) {
      physx::PxRigidBodyExt::updateMassAndInertia(*dynamic_cast<physx::PxRigidDynamic*>(actor),
                                                  1.0f);
    }
  }


  void RigidBodyComponent::Initialize()
  {
    auto& entityManager = World::GetInstance();
    auto  entityOwner = entityManager.GetEntity(owner);
    auto* transform = entityOwner->GetComponent<TransformComponent>();

    if (!transform) return;

    // Utiliser la position et la rotation globales
    XMFLOAT3 worldPosition;
    XMStoreFloat3(&worldPosition, transform->GetWorldPosition());
    XMFLOAT4 worldRotation = transform->GetWorldRotation();

    CreateActor(worldPosition, worldRotation);

    if (!actor) return;

    actor->userData = entityOwner.get();
    auto& physics = PhysicsSystem::Get();

    // Collecter les colliders de cette entité et de ses descendants
    std::vector<std::pair<ColliderComponent*, TransformComponent*>> colliderPairs;
    CollectCollidersWithTransforms(entityOwner.get(), colliderPairs);

    for (const auto& [collider, colliderTransform] : colliderPairs) {
      // Définir les flags appropriés en fonction du type (trigger ou non)
      physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE;
      if (isTrigger) {
        shapeFlags |= physx::PxShapeFlag::eTRIGGER_SHAPE;
      }
      else {
        shapeFlags |= physx::PxShapeFlag::eSIMULATION_SHAPE;
      }

      physx::PxShape* shape = physics.GetPhysics()->createShape(
        collider->GetGeometry(),
        *physics.GetMaterial(),
        true,
        shapeFlags
      );

      if (shape) {
        // Calculer la transformation locale relative au RigidBody
        physx::PxTransform localPose = ComputeColliderLocalPose(transform, colliderTransform);
        shape->setLocalPose(localPose);

        actor->attachShape(*shape);
        collider->SetShape(shape);
        shape->release();
      }
    }

    physics.GetScene()->addActor(*actor);
  }

  void RigidBodyComponent::CollectCollidersWithTransforms(
    Entity*                                                          entity,
    std::vector<std::pair<ColliderComponent*, TransformComponent*>>& colliderPairs)
  {
    // First check if this entity has both a collider and transform
    if (auto* collider = entity->GetComponent<ColliderComponent>()) {
      if (auto* transform = entity->GetComponent<TransformComponent>()) {
        // Always add if it's a valid collider, regardless of whether it's on the root or child
        colliderPairs.emplace_back(collider, transform);
      }
    }

    // Recursively check children
    if (auto* transform = entity->GetComponent<TransformComponent>()) {
      for (const auto& childId : transform->GetChildren()) {
        if (auto childEntity = World::GetInstance().GetEntity(childId)) {
          CollectCollidersWithTransforms(childEntity.get(), colliderPairs);
        }
      }
    }
  }

  physx::PxTransform RigidBodyComponent::ComputeColliderLocalPose(
    const TransformComponent* rigidBodyTransform,
    const TransformComponent* colliderTransform)
  {
    // Get world matrices
    XMMATRIX rbWorldMatrix = rigidBodyTransform->GetWorldMatrix();
    XMMATRIX colliderWorldMatrix = colliderTransform->GetWorldMatrix();

    // Calculate relative transform
    XMMATRIX rbWorldInvMatrix = XMMatrixInverse(nullptr, rbWorldMatrix);
    XMMATRIX localMatrix = XMMatrixMultiply(colliderWorldMatrix, rbWorldInvMatrix);

    // Decompose to get local transform
    XMVECTOR scale, rotationQuat, translation;
    XMMatrixDecompose(&scale, &rotationQuat, &translation, localMatrix);

    // Convert to PhysX format
    physx::PxVec3 pxTranslation;
    XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&pxTranslation), translation);

    physx::PxQuat pxRotation;
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&pxRotation), rotationQuat);

    pxRotation.normalize();

    return physx::PxTransformT<float>(pxTranslation, pxRotation);
  }


  physx::PxTransform RigidBodyComponent::ComputeLocalPose(const TransformComponent* parentTransform,
                                                          const TransformComponent* childTransform)
  {
    // Calculer la transformation locale de l'enfant par rapport au parent
    XMMATRIX parentWorldMatrix = parentTransform->GetWorldMatrix();
    XMMATRIX childWorldMatrix = childTransform->GetWorldMatrix();

    XMMATRIX parentWorldMatrixInv = XMMatrixInverse(nullptr, parentWorldMatrix);
    XMMATRIX localMatrix = XMMatrixMultiply(childWorldMatrix, parentWorldMatrixInv);

    // Décomposer la matrice locale en position et rotation
    XMVECTOR scale, rotationQuat, translation;
    XMMatrixDecompose(&scale, &rotationQuat, &translation, localMatrix);

    physx::PxVec3 pxTranslation;
    XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&pxTranslation), translation);

    physx::PxQuat pxRotation;
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&pxRotation), rotationQuat);

    return physx::PxTransform(pxTranslation, pxRotation);
  }

  void RigidBodyComponent::SetTrigger(bool enabled)
  {
    isTrigger = enabled;
    if (actor) {
      const physx::PxU32           numShapes = actor->getNbShapes();
      std::vector<physx::PxShape*> shapes(numShapes);
      actor->getShapes(shapes.data(), numShapes);

      for (auto* shape : shapes) {
        shape->setFlags(physx::PxShapeFlag::eSCENE_QUERY_SHAPE);

        if (enabled) {
          shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
          shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
        }
        else {
          shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
          shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
        }
      }
    }
  }


  void RigidBodyComponent::SetMass(const float mass) const
  {
    if (bodyType == Type::Dynamic && actor) {
      static_cast<physx::PxRigidDynamic*>(actor)->setMass(mass);
    }
  }


  void RigidBodyComponent::ApplyForce(const XMFLOAT3& force) const
  {
    if (bodyType == Type::Dynamic && actor) {
      static_cast<physx::PxRigidDynamic*>(actor)->addForce(
        physx::PxVec3(force.x, force.y, force.z)
      );
    }
  }


  void RigidBodyComponent::ApplyTorque(const XMFLOAT3& torque) const
  {
    if (bodyType == Type::Dynamic && actor) {
      static_cast<physx::PxRigidDynamic*>(actor)->addTorque(
        physx::PxVec3(torque.x, torque.y, torque.z)
      );
    }
  }

  void RigidBodyComponent::SetLinearVelocity(const XMFLOAT3& velocity) const
  {
    if (bodyType == Type::Dynamic && actor) {

      static_cast<physx::PxRigidDynamic*>(actor)->setLinearVelocity(
        physx::PxVec3(velocity.x, velocity.y, velocity.z)
      );
    }
  }

  XMFLOAT3 RigidBodyComponent::GetLinearVelocity() const
  {
    if (bodyType == Type::Dynamic && actor) {
      physx::PxVec3 physVector = static_cast<physx::PxRigidDynamic*>(actor)->getLinearVelocity();
      return {physVector.x, physVector.y, physVector.z};
    }
  }
  XMFLOAT3 RigidBodyComponent::GetAngularVelocity() const
  {
    if (bodyType == Type::Dynamic && actor) {
      physx::PxVec3 physVector = static_cast<physx::PxRigidDynamic*>(actor)->getAngularVelocity();
      return {physVector.x, physVector.y, physVector.z};
    }
  }

  void RigidBodyComponent::SetAngularVelocity(const XMFLOAT3& velocity) const
  {
    if (bodyType == Type::Dynamic && actor) {
      static_cast<physx::PxRigidDynamic*>(actor)->setAngularVelocity(
        physx::PxVec3(velocity.x, velocity.y, velocity.z)
      );
    }
  }
  void RigidBodyComponent::SyncTransformWithPhysics(const TransformComponent& transform) const
  {
    if (bodyType == Type::Static) {
      if (auto* rigidStatic = actor->is<physx::PxRigidStatic>()) {
        // Get world position and rotation
        XMFLOAT3 position;
        XMStoreFloat3(&position, transform.GetWorldPosition());

        // Ensure quaternion is normalized
        XMFLOAT4 rotation = transform.GetWorldRotation();
        XMVECTOR quatVec = XMLoadFloat4(&rotation);
        quatVec = XMQuaternionNormalize(quatVec);
        XMStoreFloat4(&rotation, quatVec);

        // Validate position values
        if (std::isnan(position.x) || std::isnan(position.y) || std::isnan(position.z) ||
          std::isinf(position.x) || std::isinf(position.y) || std::isinf(position.z)) {
          // Handle invalid position - could log error or set to default
          position = XMFLOAT3(0.0f, 0.0f, 0.0f);
        }

        // Validate rotation values
        if (std::isnan(rotation.x) || std::isnan(rotation.y) || std::isnan(rotation.z) ||
          std::isnan(rotation.w) ||
          std::isinf(rotation.x) || std::isinf(rotation.y) || std::isinf(rotation.z) || std::isinf(
            rotation.w)) {
          // Handle invalid rotation - set to identity quaternion
          rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        }

        // Create and validate PhysX transform
        physx::PxTransform pxTransform(
          physx::PxVec3(position.x, position.y, position.z),
          physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)
        );

        // Ensure the quaternion is normalized in PhysX terms
        pxTransform.q.normalize();

        // Validate the transform before setting
        if (pxTransform.isValid()) {
          rigidStatic->setGlobalPose(pxTransform);
        }
        else {
          // Handle invalid transform - could log error or set to identity
          physx::PxTransform identityTransform(
            physx::PxVec3(position.x, position.y, position.z),
            physx::PxQuat(physx::PxIdentity)
          );
          rigidStatic->setGlobalPose(identityTransform);
        }
      }
    }
    else if (bodyType == Type::Kinematic) {
      // Similar validation for kinematic bodies
      if (const auto rigidDynamic = actor->is<physx::PxRigidDynamic>()) {
        XMFLOAT3 position;
        XMStoreFloat3(&position, transform.GetWorldPosition());

        XMFLOAT4 rotation = transform.GetWorldRotation();
        XMVECTOR quatVec = XMLoadFloat4(&rotation);
        quatVec = XMQuaternionNormalize(quatVec);
        XMStoreFloat4(&rotation, quatVec);

        // Validate position and rotation as above
        if (std::isnan(position.x) || std::isnan(position.y) || std::isnan(position.z) ||
          std::isinf(position.x) || std::isinf(position.y) || std::isinf(position.z)) {
          position = XMFLOAT3(0.0f, 0.0f, 0.0f);
        }

        if (std::isnan(rotation.x) || std::isnan(rotation.y) || std::isnan(rotation.z) ||
          std::isnan(rotation.w) ||
          std::isinf(rotation.x) || std::isinf(rotation.y) || std::isinf(rotation.z) || std::isinf(
            rotation.w)) {
          rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        }

        physx::PxTransform pxTransform(
          physx::PxVec3(position.x, position.y, position.z),
          physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)
        );

        pxTransform.q.normalize();

        if (pxTransform.isValid()) {
          rigidDynamic->setKinematicTarget(pxTransform);
        }
        else {
          physx::PxTransform identityTransform(
            physx::PxVec3(position.x, position.y, position.z),
            physx::PxQuat(physx::PxIdentity)
          );
          rigidDynamic->setKinematicTarget(identityTransform);
        }
      }
    }
  }


  void RigidBodyComponent::CreateActor(const XMFLOAT3& position, const XMFLOAT4& rotation)
  {
    const auto& physics = PhysicsSystem::Get();

    physx::PxQuat pxQuat(rotation.x, rotation.y, rotation.z, rotation.w);
    pxQuat.normalize();

    physx::PxTransform transform(physx::PxVec3(position.x, position.y, position.z), pxQuat);


    switch (bodyType) {
      case Type::Static:
        actor = physics.GetPhysics()->createRigidStatic(transform);
        break;
      case Type::Dynamic:
        actor = physics.GetPhysics()->createRigidDynamic(transform);
        break;
      case Type::Kinematic:
        {
          auto* dynamicActor = physics.GetPhysics()->createRigidDynamic(transform);
          dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
          actor = dynamicActor;
        }
        break;
    }

    if (actor && isTrigger) {
      SetTrigger(true);
    }
  }


  RigidBodyComponent::RigidBodyComponent(const RigidBodyComponent& other) : Component(other),
    bodyType(other.bodyType),
    isTrigger(other.isTrigger)
  {
    if (other.actor) {
      const auto& entityManager = World::GetInstance();
      const auto& entityOwner = entityManager.GetEntity(owner);
      if (const auto* transform = entityOwner->GetComponent<TransformComponent>()) {
        XMFLOAT3 worldPosition;
        XMStoreFloat3(&worldPosition, transform->GetWorldPosition());

        XMFLOAT4 worldRotation = transform->GetWorldRotation();

        CreateActor(worldPosition, worldRotation);
        CopyActorProperties(other);
      }
    }
  }

  RigidBodyComponent& RigidBodyComponent::operator=(const RigidBodyComponent& other)
  {
    if (this != &other) {
      if (actor) {
        if (actor->getScene()) {
          actor->getScene()->removeActor(*actor);
        }
        actor->release();
        actor = nullptr;
      }

      bodyType = other.bodyType;

      if (other.actor) {
        const auto& entityManager = World::GetInstance();
        const auto& entityOwner = entityManager.GetEntity(owner);
        if (const auto* transform = entityOwner->GetComponent<TransformComponent>()) {
          XMFLOAT3 worldPosition;
          XMStoreFloat3(&worldPosition, transform->GetWorldPosition());

          XMFLOAT4 worldRotation = transform->GetWorldRotation();

          CreateActor(worldPosition, worldRotation);
          CopyActorProperties(other);
        }
      }
    }
    return *this;
  }

  RigidBodyComponent::RigidBodyComponent(RigidBodyComponent&& other) noexcept
    : Component(other)
      , bodyType(other.bodyType)
      , actor(other.actor)
      , isTrigger(other.isTrigger)
  {
    other.actor = nullptr;
  }


  RigidBodyComponent& RigidBodyComponent::operator=(RigidBodyComponent&& other) noexcept
  {
    if (this != &other) {
      if (actor) {
        if (actor->getScene()) {
          actor->getScene()->removeActor(*actor);
        }
        actor->release();
      }
      bodyType = other.bodyType;
      isTrigger = other.isTrigger; // Ajout du déplacement de isTrigger
      actor = other.actor;
      other.actor = nullptr;
    }
    return *this;
  }

  void RigidBodyComponent::Teleport(DirectX::XMFLOAT3 _position, DirectX::XMFLOAT4 _rotation)
  {


    // Récupérer les composantes du quaternion
    physx::PxQuat newOrientation(
      _rotation.x,
      _rotation.y,
      _rotation.z,
      _rotation.w
    );

    physx::PxTransform identityTransform(
      physx::PxVec3(_position.x, _position.y, _position.z),
      newOrientation
      //physx::PxQuat(angleInRadians, physx::PxVec3(_position.x, _position.y, _position.z))
    );
    actor->setGlobalPose(identityTransform);
    if (auto* dynamicActor = static_cast<physx::PxRigidDynamic*>(actor)) {
      dynamicActor->setLinearVelocity({0.0, 0.0, 0.0});
      dynamicActor->setAngularVelocity({0.0, 0.0, 0.0});
    }

  }

  float RigidBodyComponent::GetMass()
  {
    if (auto* dynamicActor = static_cast<physx::PxRigidDynamic*>(actor)) {
      return dynamicActor->getMass();
    }
    else {
      return 0.0;
    }
  }

  DirectX::XMVECTOR RigidBodyComponent::Rotate(DirectX::XMVECTOR vector, DirectX::XMFLOAT4 angle)
  {
    physx::PxQuat quaternion{angle.x, angle.y, angle.z, angle.w};
    physx::PxVec3 vectorRotated = quaternion.rotate({vector.vector4_f32[0], vector.vector4_f32[1],
      vector.vector4_f32[2]});
    return {vectorRotated.x, vectorRotated.y, vectorRotated.z};
  }


  void RigidBodyComponent::CopyActorProperties(const RigidBodyComponent& other) const
  {
    if (!actor || !other.actor) return;

    actor->setActorFlags(other.actor->getActorFlags());

    actor->setDominanceGroup(other.actor->getDominanceGroup());
    actor->setOwnerClient(other.actor->getOwnerClient());

    if (bodyType == Type::Dynamic) {
      auto*       dynamicActor = static_cast<physx::PxRigidDynamic*>(actor);
      const auto* otherDynamic = static_cast<physx::PxRigidDynamic*>(other.actor);

      dynamicActor->setMass(otherDynamic->getMass());
      dynamicActor->setLinearDamping(otherDynamic->getLinearDamping());
      dynamicActor->setAngularDamping(otherDynamic->getAngularDamping());
      dynamicActor->setMaxAngularVelocity(otherDynamic->getMaxAngularVelocity());
      dynamicActor->setMaxLinearVelocity(otherDynamic->getMaxLinearVelocity());
      dynamicActor->setRigidBodyFlags(otherDynamic->getRigidBodyFlags());

      dynamicActor->setLinearVelocity(otherDynamic->getLinearVelocity());
      dynamicActor->setAngularVelocity(otherDynamic->getAngularVelocity());

      const physx::PxRigidBodyFlags flags = otherDynamic->getRigidBodyFlags();
      if (!flags.isSet(physx::PxRigidBodyFlag::eKINEMATIC)) {
        dynamicActor->setCMassLocalPose(otherDynamic->getCMassLocalPose());
        dynamicActor->setMassSpaceInertiaTensor(otherDynamic->getMassSpaceInertiaTensor());
      }
    }

    // Copie des shapes
    const physx::PxU32           nbShapes = other.actor->getNbShapes();
    std::vector<physx::PxShape*> shapes(nbShapes);
    other.actor->getShapes(shapes.data(), nbShapes);

    const auto& physics = PhysicsSystem::Get();
    for (const auto* shape : shapes) {
      // Récupérer les matériaux
      const physx::PxU32              nbMaterials = shape->getNbMaterials();
      std::vector<physx::PxMaterial*> materials(nbMaterials);
      shape->getMaterials(materials.data(), nbMaterials);

      // Créer une nouvelle shape
      physx::PxShapeFlags shapeFlags = shape->getFlags();

      // Définir les flags en fonction de isTrigger
      if (isTrigger) {
        shapeFlags |= physx::PxShapeFlag::eTRIGGER_SHAPE;
        shapeFlags.clear(physx::PxShapeFlag::eSIMULATION_SHAPE);
      }
      else {
        shapeFlags |= physx::PxShapeFlag::eSIMULATION_SHAPE;
        shapeFlags.clear(physx::PxShapeFlag::eTRIGGER_SHAPE);
      }

      constexpr bool isExclusive = true;

      // Récupérer la géométrie
      physx::PxGeometryHolder geomHolder;
      geomHolder.storeAny(shape->getGeometry());

      if (nbMaterials > std::numeric_limits<physx::PxU16>::max()) {
        throw std::runtime_error("Number of materials exceeds the maximum allowed by PhysX.");
      }

      physx::PxShape* newShape = physics.GetPhysics()->createShape(
        geomHolder.any(),
        materials.data(),
        static_cast<physx::PxU16>(nbMaterials),
        isExclusive
      );

      if (newShape) {
        // Configurer les flags de la shape
        newShape->setFlags(shapeFlags);

        // Copier les autres propriétés de la shape
        newShape->setLocalPose(shape->getLocalPose());
        newShape->setContactOffset(shape->getContactOffset());
        newShape->setRestOffset(shape->getRestOffset());
        newShape->setSimulationFilterData(shape->getSimulationFilterData());
        newShape->setQueryFilterData(shape->getQueryFilterData());

        actor->attachShape(*newShape);
        newShape->release();
      }
    }


    // Ajouter à la scène
    if (other.actor->getScene()) {
      physics.GetScene()->addActor(*actor);
    }
  }
} // namespace PM3D
