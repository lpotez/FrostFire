#pragma once
#include <DirectXMath.h>
#include <optional>
#include <vector>
#include <algorithm>


#include "Engine/ECS/core/World.h"
#include "Engine/ECS/core/Component.h"


namespace FrostFireEngine
{
  using namespace DirectX;

  class TransformComponent : public Component {
  public:
    TransformComponent() = default;

    TransformComponent(const XMFLOAT3& pos,
                       const XMFLOAT4& rot = {0.0f, 0.0f, 0.0f, 1.0f},
                       const XMFLOAT3& scl = {1.0f, 1.0f, 1.0f})
      : position(pos), rotation(rot), scale(scl)
    {
    }

    void AddChild(EntityId childId)
    {
      if (childId == INVALID_ENTITY_ID) return;

      auto& mgr = World::GetInstance();
      auto  childEntity = mgr.GetEntity(childId);
      if (!childEntity) return;

      auto childTransform = childEntity->GetComponent<TransformComponent>();
      if (!childTransform) return;

      if (childTransform->GetParent() != INVALID_ENTITY_ID) {
        if (auto oldParentEntity = mgr.GetEntity(childTransform->GetParent())) {
          if (auto oldParentTransform = oldParentEntity->GetComponent<TransformComponent>()) {
            oldParentTransform->RemoveChildInternal(childId);
          }
        }
      }

      AddChildInternal(childId);
      childTransform->parent = owner;
      childTransform->MarkDirty();
    }

    void RemoveChild(EntityId childId)
    {
      if (childId == INVALID_ENTITY_ID) return;

      auto& mgr = World::GetInstance();
      auto  childEntity = mgr.GetEntity(childId);
      if (!childEntity) return;

      auto childTransform = childEntity->GetComponent<TransformComponent>();
      if (!childTransform) return;

      RemoveChildInternal(childId);

      if (childTransform->GetParent() == owner) {
        childTransform->parent = INVALID_ENTITY_ID;
        childTransform->MarkDirty();
      }
    }

    void SetParent(EntityId newParent)
    {
      if (parent == newParent) return;

      auto& mgr = World::GetInstance();

      if (parent != INVALID_ENTITY_ID) {
        if (auto parentEntity = mgr.GetEntity(parent)) {
          if (auto parentTransform = parentEntity->GetComponent<TransformComponent>()) {
            parentTransform->RemoveChildInternal(owner);
          }
        }
      }

      parent = newParent;
      MarkDirty();

      if (parent != INVALID_ENTITY_ID) {
        if (auto parentEntity = mgr.GetEntity(parent)) {
          if (auto parentTransform = parentEntity->GetComponent<TransformComponent>()) {
            parentTransform->AddChildInternal(owner);
          }
        }
      }
    }

    EntityId GetParent() const
    {
      return parent;
    }
    const std::vector<EntityId>& GetChildren() const
    {
      return children;
    }

    // Set local position relative to parent
    TransformComponent& SetPosition(const XMFLOAT3& pos)
    {
      position = pos;
      MarkDirty();
      return *this;
    }

    // Set local rotation relative to parent
    TransformComponent& SetRotation(const XMFLOAT4& rot)
    {
      rotation = rot;
      NormalizeRotation();
      return *this;
    }

    // Utility method to set rotation from Euler angles (in radians)
    TransformComponent& SetRotationEuler(const XMFLOAT3& euler)
    {
      XMVECTOR quatRotation = XMQuaternionRotationRollPitchYaw(euler.x, euler.y, euler.z);
      XMStoreFloat4(&rotation, quatRotation);
      NormalizeRotation();
      return *this;
    }

    // Set local scale relative to parent
    TransformComponent& SetScale(const XMFLOAT3& scl);

    const XMFLOAT3& GetPosition() const
    {
      return position;
    } // Local position
    const XMFLOAT4& GetRotation() const
    {
      return rotation;
    } // Local rotation
    const XMFLOAT3& GetScale() const
    {
      return scale;
    } // Local scale

    // Get rotation in Euler angles (in radians)
    XMFLOAT3 GetRotationEuler() const
    {
      XMFLOAT4 q = rotation;
      float    ysqr = q.y * q.y;

      // Roll (X-axis rotation)
      float t0 = +2.0f * (q.w * q.x + q.y * q.z);
      float t1 = +1.0f - 2.0f * (q.x * q.x + ysqr);
      float roll = std::atan2f(t0, t1);

      // Pitch (Y-axis rotation)
      float t2 = +2.0f * (q.w * q.y - q.z * q.x);
      t2 = std::clamp(t2, -1.0f, 1.0f);
      float pitch = std::asinf(t2);

      // Yaw (Z-axis rotation)
      float t3 = +2.0f * (q.w * q.z + q.x * q.y);
      float t4 = +1.0f - 2.0f * (ysqr + q.z * q.z);
      float yaw = std::atan2f(t3, t4);

      return XMFLOAT3(roll, pitch, yaw);
    }

    // Get local transformation matrix
    XMMATRIX GetLocalMatrix() const
    {
      if (!isDirty && cachedLocalMatrix) return cachedLocalMatrix.value();

      XMMATRIX matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
      XMMATRIX matRotation = XMMatrixRotationQuaternion(XMLoadFloat4(&rotation));
      XMMATRIX matTranslation = XMMatrixTranslation(position.x, position.y, position.z);

      // Local transformations: Scale * Rotation * Translation
      cachedLocalMatrix = matScale * matRotation * matTranslation;
      isDirty = false;
      return cachedLocalMatrix.value();
    }

    // Get world transformation matrix
    XMMATRIX GetWorldMatrix() const
    {
      if (!isDirty && cachedWorldMatrix) return cachedWorldMatrix.value();

      XMMATRIX localMatrix = GetLocalMatrix();

      if (parent != INVALID_ENTITY_ID) {
        if (auto parentEntity = World::GetInstance().GetEntity(parent)) {
          if (auto parentTransform = parentEntity->GetComponent<TransformComponent>()) {
            XMMATRIX parentWorldMatrix = parentTransform->GetWorldMatrix();
            // Apply local transformations before parent transformations
            cachedWorldMatrix = localMatrix * parentWorldMatrix;
            isDirty = false;
            return cachedWorldMatrix.value();
          }
        }
      }

      cachedWorldMatrix = localMatrix;
      isDirty = false;
      return cachedWorldMatrix.value();
    }

    // Get forward direction in world space
    XMVECTOR GetForward() const
    {
      XMMATRIX worldMatrix = GetWorldMatrix();
      XMVECTOR forward = XMVector3Normalize(worldMatrix.r[2]); // Third row
      return forward;
    }

    // Get right direction in world space
    XMVECTOR GetRight() const
    {
      XMMATRIX worldMatrix = GetWorldMatrix();
      XMVECTOR right = XMVector3Normalize(worldMatrix.r[0]); // First row
      return right;
    }

    // Get up direction in world space
    XMVECTOR GetUp() const
    {
      XMMATRIX worldMatrix = GetWorldMatrix();
      XMVECTOR up = XMVector3Normalize(worldMatrix.r[1]); // Second row
      return up;
    }

    // Get world position
    XMVECTOR GetWorldPosition() const
    {
      XMMATRIX worldMatrix = GetWorldMatrix();
      return worldMatrix.r[3]; // Fourth row
    }

    XMFLOAT4 GetWorldRotation() const
    {
      XMVECTOR worldRotationQuat = GetWorldRotationQuaternion();
      XMFLOAT4 worldRotation;
      XMStoreFloat4(&worldRotation, worldRotationQuat);
      return worldRotation;
    }

    // Set world position
    void SetWorldPosition(const XMFLOAT3& worldPos)
    {
      XMVECTOR desiredWorldPos = XMLoadFloat3(&worldPos);

      if (parent != INVALID_ENTITY_ID) {
        auto parentTransform = World::GetInstance().GetEntity(parent)->GetComponent<
          TransformComponent>();
        if (parentTransform) {
          XMMATRIX parentWorldMatrix = parentTransform->GetWorldMatrix();
          XMMATRIX parentWorldMatrixInv = XMMatrixInverse(nullptr, parentWorldMatrix);
          XMVECTOR localPos = XMVector3TransformCoord(desiredWorldPos, parentWorldMatrixInv);
          XMStoreFloat3(&position, localPos);
        }
      }
      else {
        XMStoreFloat3(&position, desiredWorldPos);
      }
      MarkDirty();
    }

    void SetWorldRotation(const XMFLOAT4& worldRot)
    {
      XMVECTOR desiredWorldRot = XMLoadFloat4(&worldRot);

      if (parent != INVALID_ENTITY_ID) {
        if (auto parentTransform = World::GetInstance().GetEntity(parent)->GetComponent<
          TransformComponent>()) {
          XMVECTOR invParentRotation = XMQuaternionInverse(
            parentTransform->GetWorldRotationQuaternion());
          XMVECTOR localRot = XMQuaternionMultiply(invParentRotation, desiredWorldRot);
          XMStoreFloat4(&rotation, localRot);
        }
      }
      else {
        XMStoreFloat4(&rotation, desiredWorldRot);
      }
      NormalizeRotation();
    }


    // Make the object look at a target position
    TransformComponent& LookAt(const XMFLOAT3& target)
    {
      XMVECTOR currentPos = GetWorldPosition();
      XMVECTOR targetPos = XMLoadFloat3(&target);

      XMVECTOR direction = XMVectorSubtract(targetPos, currentPos);

      if (XMVector3NearEqual(direction, XMVectorZero(), XMVectorReplicate(1e-6f))) {
        return *this;
      }

      direction = XMVector3Normalize(direction);

      XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
      XMMATRIX lookAtMatrix = XMMatrixLookToLH(currentPos, direction, up);
      XMMATRIX rotationMatrix = XMMatrixTranspose(lookAtMatrix);

      XMVECTOR quatRotation = XMQuaternionRotationMatrix(rotationMatrix);

      // Convert world rotation to local rotation
      if (parent != INVALID_ENTITY_ID) {
        auto parentTransform = World::GetInstance().GetEntity(parent)->GetComponent<
          TransformComponent>();
        if (parentTransform) {
          XMVECTOR parentRotationQuat = parentTransform->GetWorldRotationQuaternion();
          XMVECTOR invParentRotation = XMQuaternionInverse(parentRotationQuat);
          quatRotation = XMQuaternionMultiply(invParentRotation, quatRotation);
        }
      }

      XMStoreFloat4(&rotation, quatRotation);
      MarkDirty();

      return *this;
    }

  private:
    void NormalizeRotation()
    {
      XMVECTOR quatVec = XMLoadFloat4(&rotation);
      quatVec = XMQuaternionNormalize(quatVec);
      XMStoreFloat4(&rotation, quatVec);
      MarkDirty();
    }

    void AddChildInternal(EntityId childId)
    {
      if (childId != INVALID_ENTITY_ID && std::find(children.begin(), children.end(), childId) ==
        children.end()) {
        children.push_back(childId);
      }
    }

    void RemoveChildInternal(EntityId childId)
    {
      auto it = std::find(children.begin(), children.end(), childId);
      if (it != children.end()) {
        children.erase(it);
      }
    }

    void MarkDirty() const;


    // Helper function to get world rotation quaternion
    XMVECTOR GetWorldRotationQuaternion() const
    {
      XMVECTOR localRotationQuat = XMLoadFloat4(&rotation);

      if (parent != INVALID_ENTITY_ID) {
        auto parentEntity = World::GetInstance().GetEntity(parent);
        if (parentEntity) {
          auto parentTransform = parentEntity->GetComponent<TransformComponent>();
          if (parentTransform) {
            XMVECTOR parentWorldRotationQuat = parentTransform->GetWorldRotationQuaternion();
            return XMQuaternionMultiply(localRotationQuat, parentWorldRotationQuat);
          }
        }
      }

      return localRotationQuat;
    }

    XMFLOAT3                        position{}; // Local position relative to parent
    XMFLOAT4                        rotation{0.0f, 0.0f, 0.0f, 1.0f}; // Local rotation (quaternion)
    XMFLOAT3                        scale{1.0f, 1.0f, 1.0f}; // Local scale
    EntityId                        parent{INVALID_ENTITY_ID};
    std::vector<EntityId>           children;
    mutable bool                    isDirty{true};
    mutable std::optional<XMMATRIX> cachedLocalMatrix;
    mutable std::optional<XMMATRIX> cachedWorldMatrix;
  };
}
