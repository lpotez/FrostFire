#pragma once

#include <iostream>

#include "ECS/components/scripts/ScriptComponent.h"
#include "CarDynamicScript.h"

namespace FrostFireEngine
{
  class ScriptSpeedometer final : public ScriptComponent {
  public:
    ScriptSpeedometer() = default;

    ScriptSpeedometer(const EntityId& vehiculeId) : vehiculeId_(vehiculeId),
                                                    speed_(0.0f, 0.0f, 0.0f),
                                                    vel_(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f)),
                                                    norm_(0.0f),
                                                    transformIndicator(nullptr)
    {
      rbVehicule_ = World::GetInstance().GetEntity(vehiculeId_)->GetComponent<RigidBodyComponent>();
    }


    void Start() override
    {
      transformIndicator = GetEntity()->GetComponent<TransformComponent>();
    }

    void Update(float deltaTime) override
    {
      speed_ = rbVehicule_->GetLinearVelocity();
      vel_ = {speed_.x, speed_.y, speed_.z};
      norm_ = sqrt(speed_.x * speed_.x + speed_.y * speed_.y + speed_.z * speed_.z);

      float maxSpeed = 32.0f;
      float maxAngle = XM_2PI;

      // Conversion de la vitesse en angle
      float speedNormalized = std::clamp((norm_) / maxSpeed, 0.0f, 1.0f);
      currentAngle = speedNormalized * maxAngle;

      XMVECTOR rotationAxis = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
      XMVECTOR rotationQuaternion = XMQuaternionRotationAxis(rotationAxis, currentAngle);

      XMFLOAT4 newRotation;
      XMStoreFloat4(&newRotation, rotationQuaternion);

      transformIndicator->SetRotationEuler({0, 0, 0.72f * currentAngle});
    }

  private:
    EntityId            vehiculeId_;
    RigidBodyComponent* rbVehicule_;
    TransformComponent* transformIndicator;

    XMFLOAT3 speed_;
    XMVECTOR vel_;
    float    norm_;
    float    currentAngle = 0.0f;
    float    tempsEcoule = 0.0f;
  };
}
