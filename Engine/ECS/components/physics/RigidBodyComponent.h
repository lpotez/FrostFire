#pragma once
#include "Engine/ECS/core/Component.h"
#include <DirectXMath.h>
#include <vector>
#include <PxPhysicsAPI.h>
namespace physx
{
  class PxRigidActor;
}

namespace FrostFireEngine
{
  // Forward declarations
  class PhysicsSystem;
  class ColliderComponent;
  class TransformComponent;

  class RigidBodyComponent : public Component
  {
  public:
    enum class Type
    {
      Static,
      Dynamic,
      Kinematic
    };

    // Constructeur par défaut avec type optionnel
    explicit RigidBodyComponent(Type type = Type::Dynamic, bool isTrigger = false);
    ~RigidBodyComponent() override;
    void UpdateMassProperties() const;

    // Support pour copie profonde de l'état physique
    RigidBodyComponent(const RigidBodyComponent& other);
    RigidBodyComponent& operator=(const RigidBodyComponent& other);
    RigidBodyComponent(RigidBodyComponent&& other) noexcept;
    RigidBodyComponent& operator=(RigidBodyComponent&& other) noexcept;

    void Teleport(DirectX::XMFLOAT3 _position, DirectX::XMFLOAT4 _rotation);

    float GetMass();

    DirectX::XMVECTOR Rotate(DirectX::XMVECTOR vector, DirectX::XMFLOAT4 angle);

    void Initialize();

    // Méthodes pour la manipulation physique (uniquement pour les corps dynamiques)
    void SetMass(float mass) const;
    void ApplyForce(const DirectX::XMFLOAT3& force) const;
    void ApplyTorque(const DirectX::XMFLOAT3& torque) const;
    void SetLinearVelocity(const DirectX::XMFLOAT3& velocity) const;
    DirectX::XMFLOAT3  GetLinearVelocity() const;
    DirectX::XMFLOAT3 GetAngularVelocity() const;
    void SetAngularVelocity(const DirectX::XMFLOAT3& velocity) const;


    void SyncTransformWithPhysics(const TransformComponent& transform) const;

    void SetTrigger(bool enabled);
    bool IsTrigger() const { return isTrigger; }

    // Accesseurs
    physx::PxRigidActor* GetActor() const { return actor; }
    Type GetType() const { return bodyType; }

  private:
    // Utilitaire interne pour la copie d'acteur
    void CopyActorProperties(const RigidBodyComponent& other) const;
    void CreateActor(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& rotation);

    static physx::PxTransform ComputeLocalPose(const TransformComponent* parentTransform,
                                               const TransformComponent* childTransform);

    static void CollectCollidersWithTransforms(Entity* entity,
                                            std::vector<std::pair<ColliderComponent*, TransformComponent
                                                                  *>>& colliderPairs);
    static physx::PxTransform ComputeColliderLocalPose(const TransformComponent* rigidBodyTransform,
                                                       const TransformComponent* colliderTransform);


    Type bodyType;
    physx::PxRigidActor* actor{nullptr};
    bool isTrigger{false};
  };
}
