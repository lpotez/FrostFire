#pragma once
#include "stdafx.h"
#include "InputManager.h"

using namespace DirectX;

namespace FrostFireEngine
{
  class FreeCameraScript : public ScriptComponent {
  public:
    struct CameraConfig {
      float moveSpeed_{10.0f};
      float rotationSpeed_{1.5f};
      float sensitivity_{0.1f};
      bool  invertY = false;
    };

    FreeCameraScript()
    {
    }

    void Awake() override
    {
      transform = GetEntity()->GetComponent<TransformComponent>();
      freePosition = XMLoadFloat3(&transform->GetPosition());
    }

    void Update(float elapsedTime) override
    {
      InputManager& rGestionnaireDeSaisie = InputManager::GetInstance();

      XMVECTOR forward = transform->GetForward();
      XMVECTOR right = transform->GetRight();
      XMVECTOR upVector = XMVectorSet(0, 1, 0, 0);
      XMVECTOR currentRotation = XMLoadFloat4(&transform->GetRotation());

      float movementSpeed = config.moveSpeed_ * (XM_PI * 2.0f / 7.0f * elapsedTime);
      float rotationSpeed = config.rotationSpeed_ * (XM_PI * 2.0f / 7.0f * elapsedTime);

      //Déplacements de la caméra
      if (rGestionnaireDeSaisie.IsKeyPressed(DIK_E)) {
        freePosition = XMVectorAdd(freePosition, XMVectorScale(upVector, movementSpeed));
      }
      if (rGestionnaireDeSaisie.IsKeyPressed(DIK_Q)) {
        freePosition = XMVectorSubtract(freePosition, XMVectorScale(upVector, movementSpeed));
      }

      if (rGestionnaireDeSaisie.IsKeyPressed(DIK_W)) {
        XMVECTOR forwardXZ = XMVectorSet(XMVectorGetX(forward), XMVectorGetY(forward),
                                         XMVectorGetZ(forward), 0.0f);
        freePosition = XMVectorAdd(freePosition, XMVectorScale(forwardXZ, movementSpeed));
      }

      if (rGestionnaireDeSaisie.IsKeyPressed(DIK_S)) {
        XMVECTOR forwardXZ = XMVectorSet(XMVectorGetX(forward), XMVectorGetY(forward),
                                         XMVectorGetZ(forward), 0.0f);
        freePosition = XMVectorSubtract(freePosition, XMVectorScale(forwardXZ, movementSpeed));
      }

      if (rGestionnaireDeSaisie.IsKeyPressed(DIK_A)) {
        XMVECTOR rightXZ = XMVectorSet(XMVectorGetX(right), 0.0f, XMVectorGetZ(right), 0.0f);
        freePosition = XMVectorSubtract(freePosition, XMVectorScale(rightXZ, movementSpeed));
      }

      if (rGestionnaireDeSaisie.IsKeyPressed(DIK_D)) {
        XMVECTOR rightXZ = XMVectorSet(XMVectorGetX(right), 0.0f, XMVectorGetZ(right), 0.0f);
        freePosition = XMVectorAdd(freePosition, XMVectorScale(rightXZ, movementSpeed));
      }

      // Reset la caméra libre = R
      if (rGestionnaireDeSaisie.IsKeyPressed(DIK_R)) {
        freePosition = {0.0f, 0.0f, 0.0f};
      }

      // Gestion du mode UI :
      // Si on est en mode UI, on n'autorise la rotation de la caméra que si le bouton droit de la souris est enfoncé.
      bool inUIMode = rGestionnaireDeSaisie.IsInUIMode();
      bool rightMouseDown = rGestionnaireDeSaisie.IsMouseButtonPressed(1);
      // 1 = bouton droit souris (à adapter selon votre input manager)

      // Contrôle de la vitesse avec la molette
      int scrollDelta = rGestionnaireDeSaisie.GetMouseDZ();
      if (scrollDelta != 0) {
        config.moveSpeed_ += scrollDelta * 0.1f;
        if (config.moveSpeed_ < 0.1f) config.moveSpeed_ = 0.1f;
        // Eviter une vitesse trop basse ou négative
      }

      // Si on est en UI mode et qu'on ne clique pas droit, on ne fait aucun mouvement (on return)
      if (inUIMode && !rightMouseDown) {
        // On laisse juste la vitesse pouvoir être modifiée par la molette
        // mais on ne fait pas de rotation de caméra si clic droit pas enfoncé
        XMFLOAT3 freePositionFloat;
        XMStoreFloat3(&freePositionFloat, freePosition);
        transform->SetPosition(freePositionFloat);

        XMFLOAT4 newRotation;
        XMStoreFloat4(&newRotation, currentRotation);
        transform->SetRotation(newRotation);
        return;
      }

      // Mouvements souris (rotation) uniquement si pas en mode UI
      // ou si en mode UI + clic droit enfoncé
      float mouseSensitivity = config.sensitivity_;
      if (rGestionnaireDeSaisie.HasMouseMoved()) {
        float mouseX = rGestionnaireDeSaisie.GetMouseDX() * mouseSensitivity * elapsedTime;
        float mouseY = rGestionnaireDeSaisie.GetMouseDY() * mouseSensitivity * elapsedTime;

        // Limiter les deltas pour éviter les mouvements trop brusques
        mouseX = std::clamp(mouseX, -0.1f, 0.1f);
        mouseY = std::clamp(mouseY, -0.1f, 0.1f);

        XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);
        XMVECTOR currentRight = transform->GetRight();
        XMVECTOR currentForward = transform->GetForward();

        float dotProduct = XMVectorGetY(currentForward);
        float currentVerticalAngle = asinf(dotProduct);

        const float MAX_ANGLE = XM_PIDIV2 * 0.89f; // Environ 80 degrés
        float       projectedAngle = currentVerticalAngle - mouseY;

        if (projectedAngle > MAX_ANGLE) {
          mouseY = currentVerticalAngle - MAX_ANGLE;
        }
        else if (projectedAngle < -MAX_ANGLE) {
          mouseY = currentVerticalAngle + MAX_ANGLE;
        }

        // Rotation horizontale
        XMVECTOR horizontalRotation = XMQuaternionRotationAxis(worldUp, mouseX);
        currentRotation = XMQuaternionMultiply(currentRotation, horizontalRotation);

        // Rotation verticale
        if (mouseY != 0.0f) {
          currentRight = XMVector3Normalize(
            XMVector3Cross(worldUp, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), currentRotation))
          );

          XMVECTOR verticalRotation = XMQuaternionRotationAxis(currentRight, mouseY);
          currentRotation = XMQuaternionMultiply(currentRotation, verticalRotation);
        }

        currentRotation = XMQuaternionNormalize(currentRotation);
      }

      // Mise à jour finale
      XMFLOAT3 freePositionFloat;
      XMStoreFloat3(&freePositionFloat, freePosition);

      currentRotation = XMQuaternionNormalize(currentRotation);
      XMFLOAT4 newRotation;
      XMStoreFloat4(&newRotation, currentRotation);

      transform->SetPosition(freePositionFloat);
      transform->SetRotation(newRotation);
    }

    CameraConfig GetConfig() const
    {
      return config;
    }

    void SetConfig(const float& moveSpeed, const float& rotationSpeed, const float& sensitivity)
    {
      config.moveSpeed_ = moveSpeed;
      config.rotationSpeed_ = rotationSpeed;
      config.sensitivity_ = sensitivity;
    }

  private:
    XMVECTOR            freePosition;
    CameraConfig        config;
    TransformComponent* transform = nullptr;
  };
}
