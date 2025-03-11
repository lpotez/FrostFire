#pragma once
#include "transform/TransformComponent.h"
#include "engine/DispositifD3D11.h"

using namespace DirectX;

namespace FrostFireEngine
{
  class CameraComponent : public Component {
  public:
    CameraComponent() = default;

    virtual ~CameraComponent()
    {
      delete matView_;
      delete matProj_;
      delete matViewProj_;
    }

    CameraComponent(float    fov,
                    float    aspectRatio,
                    float    nearPlane,
                    float    farPlane,
                    EntityId cameraId)
      : fov_(fov),
        aspectRatio_(aspectRatio),
        nearPlane_(nearPlane),
        farPlane_(farPlane),
        matProj_(new XMMATRIX),
        matView_(new XMMATRIX),
        matViewProj_(new XMMATRIX),
        matOrtho_(new XMMATRIX),
        cameraId_(cameraId)
    {
    }

    CameraComponent(const CameraComponent& other)
      : fov_(other.fov_),
        aspectRatio_(other.aspectRatio_),
        nearPlane_(other.nearPlane_),
        farPlane_(other.farPlane_),
        matView_(new XMMATRIX(*other.matView_)),
        matProj_(new XMMATRIX(*other.matProj_)),
        matViewProj_(new XMMATRIX(*other.matViewProj_)),
        matOrtho_(new XMMATRIX(*other.matOrtho_)),
        cameraId_(other.cameraId_)
    {
      UpdateProjMatrix();
    }

    CameraComponent& operator=(const CameraComponent& other)
    {
      if (this != &other) {
        fov_ = other.fov_;
        aspectRatio_ = other.aspectRatio_;
        nearPlane_ = other.nearPlane_;
        farPlane_ = other.farPlane_;

        *matView_ = *other.matView_;
        *matProj_ = *other.matProj_;
        *matViewProj_ = *other.matViewProj_;
        *matOrtho_ = *other.matOrtho_;

        cameraId_ = other.cameraId_;
      }
      return *this;
    }

    void LookAt(const XMFLOAT3& position, const XMFLOAT3& target, const XMVECTOR& up) const
    {
      *matView_ = XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&target), up);
      UpdateViewProjMatrix();
    }

    void UpdateProjMatrix() const
    {
      *matProj_ = XMMatrixPerspectiveFovLH(fov_, aspectRatio_, nearPlane_, farPlane_);
      UpdateViewProjMatrix();
    }

    void UpdateOrthoMatrix(const DispositifD3D11* device) const
    {
      float width = device->GetViewportWidth();
      float height = device->GetViewportHeight();
      *matOrtho_ =
      XMMatrixOrthographicOffCenterLH(0.0f, width, height, 0.0f, nearPlane_, farPlane_);
    }

    void UpdateViewMatrix() const
    {
      auto cameraEntity = World::GetInstance().GetEntity(cameraId_);
      auto cameraTransform = cameraEntity->GetComponent<TransformComponent>();

      XMVECTOR cameraPos = XMLoadFloat3(&cameraTransform->GetPosition());
      XMVECTOR cameraDir = cameraTransform->GetForward();
      XMVECTOR cameraUp = cameraTransform->GetUp();

      *matView_ = XMMatrixLookAtLH(cameraPos, cameraDir + cameraPos, cameraUp);
      UpdateViewProjMatrix();
    }

    void UpdateViewProjMatrix() const
    {
      *matViewProj_ = (*matView_) * (*matProj_);
    }

    // Getters
    [[nodiscard]] const XMMATRIX& GetViewMatrix() const
    {
      return *matView_;
    }
    [[nodiscard]] const XMMATRIX& GetProjMatrix() const
    {
      return *matProj_;
    }
    [[nodiscard]] const XMMATRIX& GetViewProjMatrix() const
    {
      return *matViewProj_;
    }
    [[nodiscard]] const XMMATRIX& GetOrthoMatrix() const
    {
      return *matOrtho_;
    }

    [[nodiscard]] float GetFov() const
    {
      return fov_;
    }
    [[nodiscard]] float GetAspectRatio() const
    {
      return aspectRatio_;
    }
    [[nodiscard]] float GetNearPlane() const
    {
      return nearPlane_;
    }
    [[nodiscard]] float GetFarPlane() const
    {
      return farPlane_;
    }

    // Setters
    void SetCameraValues(float fov,
                         float aspectRatio,
                         float nearPlane,
                         float farPlane)
    {
      fov_ = fov;
      aspectRatio_ = aspectRatio;
      nearPlane_ = nearPlane;
      farPlane_ = farPlane;
      UpdateProjMatrix();
    }

    void SetFov(const float& fov)
    {
      fov_ = fov;
      UpdateProjMatrix();
    }
    void SetAspectRatio(const float& aspectRatio)
    {
      aspectRatio_ = aspectRatio;
      UpdateProjMatrix();
    }
    void SetNearPlane(const float& nearPlane)
    {
      nearPlane_ = nearPlane;
      UpdateProjMatrix();
    }
    void SetFarPlane(const float& farPlane)
    {
      farPlane_ = farPlane;
      UpdateProjMatrix();
    }

  private:
    float     fov_;
    float     aspectRatio_;
    float     nearPlane_;
    float     farPlane_;
    XMMATRIX* matView_;
    XMMATRIX* matProj_;
    XMMATRIX* matViewProj_;
    XMMATRIX* matOrtho_;

    EntityId cameraId_;
  };
}
