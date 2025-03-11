#pragma once
#include "FreeCameraScript.h"

namespace FrostFireEngine
{
  class CameraScript final : public ScriptComponent {
  public:
    int timer = 0;

    CameraScript() = default;

    CameraScript(const EntityId& cameraTPSId,
                 const EntityId& cameraFPSId,
                 const EntityId& freeCameraId,
                 const EntityId& targetId)
      : cameraTPSId_(cameraTPSId), cameraFPSId_(cameraFPSId),
        freeCameraId_(freeCameraId), targetId_(targetId),
        cameraPreviousId_(cameraTPSId_)
    {
    }

    void SetCameraActive(const EntityId& cameraId) const
    {
      const auto& world = World::GetInstance();
      if (auto* cameraSystem = world.GetSystem<CameraSystem>()) {
        cameraSystem->SetActiveCamera(cameraId);

        // Handle free camera script enabling/disabling
        if (const auto freeCameraEntity = world.GetEntity(freeCameraId_)) {
          if (const auto freeCameraScript = freeCameraEntity->GetComponent<FreeCameraScript>()) {
            freeCameraScript->SetEnabled(cameraId == freeCameraId_);
          }
        }
      }
    }
    void SetTarget(const EntityId& targetId)
    {
      targetId_ = targetId;
    }

    void UpdateToFPSCam()
    {
      SetCameraActive(cameraFPSId_);
    }

    void UpdateToTPSCam()
    {
      SetCameraActive(cameraTPSId_);
    }

    void Update(float) override
    {
      const auto& world = World::GetInstance();
      const auto* cameraSystem = world.GetSystem<CameraSystem>();
      if (!cameraSystem) {
        throw std::runtime_error("CameraSystem not found");
      }

      const auto activeCameraEntity = cameraSystem->GetActiveCameraEntity();
      if (!activeCameraEntity) {
        throw std::runtime_error("No active camera set in CameraSystem");
      }

      const EntityId activeCameraId = activeCameraEntity->GetId();

      const auto cameraTransform = activeCameraEntity->GetComponent<TransformComponent>();
      const auto targetEntity = world.GetEntity(targetId_);
      const auto targetTransform = targetEntity->GetComponent<TransformComponent>();

      const XMVECTOR targetPositionVec = targetTransform->GetWorldPosition();
      XMFLOAT3       targetPosition;
      XMStoreFloat3(&targetPosition, targetPositionVec);

      const XMVECTOR targetRightFPSVec = XMVectorAdd(targetTransform->GetRight(),
                                                     XMVectorSet(0.0f, 0.75f, 0.0f, 0.0f));

      const XMVECTOR targetRightVec = XMVectorAdd(-targetTransform->GetRight(),
                                                  XMVectorSet(0.0f, 0.25f, 0.0f, 0.0f));
      const XMVECTOR targetRightVecScale = XMVectorScale(targetRightVec, 8.0f);

      if (activeCameraId == cameraTPSId_) {
        float smoothingFactor = 0.1f;

        XMVECTOR offset = XMVectorSet(0.0f, 1.0f, 3.0f, 0.0f);
        XMVECTOR newPosCameraVec = XMVectorAdd(targetRightVecScale, targetPositionVec);
        XMFLOAT3 targetPosRel = targetTransform->GetPosition();

        XMVECTOR newCameraPositionVec = XMVectorLerp(newPosCameraVec, targetPositionVec,
                                                     smoothingFactor);
        XMFLOAT3 newCameraPosition;
        XMStoreFloat3(&newCameraPosition, newCameraPositionVec);
        cameraTransform->SetPosition(newCameraPosition);
        cameraTransform->LookAt(targetPosition);
      }

      else if (activeCameraId == cameraFPSId_) {
        XMFLOAT3 cameraPos = {
          targetPosition.x,
          targetPosition.y + 0.75f,
          targetPosition.z
        };

        XMVECTOR dir = XMVectorAdd(targetRightFPSVec, targetPositionVec);

        XMFLOAT3 dirFloat;
        XMStoreFloat3(&dirFloat, dir);

        cameraTransform->SetPosition(cameraPos);
        cameraTransform->LookAt(dirFloat);
      }
      else if (activeCameraId == freeCameraId_) {
      }

      if (PauseManagerSystem::Get().IsPaused()) {
        return;
      }

      if (timer <= 0 && InputManager::GetInstance().IsKeyPressed(DIK_F5)) {
        if (activeCameraId == cameraTPSId_) {
          UpdateToFPSCam();
          timer = 30;
        }
        else if (activeCameraId == cameraFPSId_) {
          UpdateToTPSCam();
          timer = 30;
        }
        else if (activeCameraId == freeCameraId_) {
          SetCameraActive(cameraPreviousId_);

          timer = 30;
        }
      }

      #ifdef _DEBUG
      if (timer <= 0 && InputManager::GetInstance().IsKeyPressed(DIK_TAB)) {
        if (activeCameraId != freeCameraId_) {
          cameraPreviousId_ = activeCameraId;

          if (const auto previousCameraEntity = world.GetEntity(cameraPreviousId_)) {
            if (const auto scriptComponent = previousCameraEntity->GetComponent<CameraScript>()) {
              scriptComponent->SetEnabled(false);
            }
          }
          SetCameraActive(freeCameraId_);
          timer = 30;
        }
        else {
          if (const auto previousCameraEntity = world.GetEntity(cameraPreviousId_)) {
            if (const auto scriptComponent = previousCameraEntity->GetComponent<CameraScript>()) {
              scriptComponent->SetEnabled(true);
            }
          }
          SetCameraActive(cameraPreviousId_);
          timer = 30;
        }
      }
      #endif

      timer--;
    }

  private:
    EntityId cameraTPSId_;
    EntityId cameraFPSId_;
    EntityId freeCameraId_;
    EntityId targetId_;

    EntityId cameraPreviousId_;
  };
}
