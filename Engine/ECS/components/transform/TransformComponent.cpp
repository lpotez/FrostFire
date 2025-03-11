#include "TransformComponent.h"

#include "Engine/ECS/components/physics/ColliderComponent.h"
#include "Engine/ECS/components/physics/RigidBodyComponent.h"
#include "Engine/ECS/components/rendering/PBRRenderer.h"

namespace FrostFireEngine
{
  TransformComponent& TransformComponent::SetScale(const XMFLOAT3& scl)
  {
    scale = scl;
    MarkDirty();

    if (owner == INVALID_ENTITY_ID) return *this;
    const auto ownerEntity = World::GetInstance().GetEntity(this->owner);
    if (auto collider = ownerEntity->GetComponent<ColliderComponent>()) {
      collider->UpdateScale(scale);
    }

    return *this;
  }
  void TransformComponent::MarkDirty() const
  {
    if (!isDirty) {
      isDirty = true;
      cachedLocalMatrix.reset();
      cachedWorldMatrix.reset();

      if (owner != INVALID_ENTITY_ID) {
        auto ownerEntity = World::GetInstance().GetEntity(owner);
        if (auto rigidBody = ownerEntity->GetComponent<RigidBodyComponent>()) {
          rigidBody->SyncTransformWithPhysics(*this);
        }
      }

      // Mise à jour de l'entité dans l'octree si elle possède un renderer
      if (owner != INVALID_ENTITY_ID) {
        auto ownerEntity = World::GetInstance().GetEntity(owner);
        if (ownerEntity && ownerEntity->GetComponent<BaseRendererComponent>()) {
          World::GetInstance().UpdateOctreeEntity(owner);
        }
      }

      for (auto childId : children) {
        if (auto childEntity = World::GetInstance().GetEntity(childId)) {
          if (auto childTransform = childEntity->GetComponent<TransformComponent>()) {
            childTransform->MarkDirty();
          }
        }
      }
    }
  }
}
