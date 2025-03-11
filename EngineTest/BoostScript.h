#pragma once

#include "ECS/components/rendering/PBRRenderer.h"
#include "InputManager.h"
#include <vector>


#include "CheckpointScript.h"
#include "ECS/components/physics/RigidBodyComponent.h"
#include "ECS/components/rendering/UIRendererComponent.h"
#include "ECS/systems/PauseManagerSystem.h"
#include "ECS/systems/PhysicsSystem.h"
#include "CarDynamicScript.h"


namespace FrostFireEngine
{
  class BoostScript final : public ScriptComponent {

    float boostCoef = 0.0;
    int boostDuration = 0;

  public:
    BoostScript(float _coef, int _duration) : boostCoef(_coef), boostDuration(_duration)
    {
    }
    void Awake() override
    {

    }

    void Start() override
    {
    }

    void Update(float deltaTime) override
    {

    }

    void OnCollisionEnter(const EntityId other) override
    {
      //std::cout << "MyScript OnCollisionEnter with Entity " << other << "\n";
      int j = 0;
    }

    void OnCollisionStay(const EntityId other) override
    {
      //std::cout << "MyScript OnCollisionStay with Entity " << other << "\n";
    }

    void OnCollisionExit(const EntityId other) override
    {
      //std::cout << "MyScript OnCollisionExit with Entity " << other << "\n";
    }

    void OnTriggerEnter(const EntityId other) override
    {
      /*
      //std::cout << "MyScript OnTriggerEnter with Entity " << other << "\n";
      auto              _other = World::GetInstance().GetEntity(other);
      if (auto script = _other->GetComponent<CarDynamicScript>()) {
        script->SetBoost(boostCoef, boostDuration);
      }
      */
    }

    void OnTriggerStay(const EntityId other) override
    {
      //std::cout << "MyScript OnTriggerStay with Entity " << other << "\n";
    }

    void OnTriggerExit(const EntityId other) override
    {
      //std::cout << "MyScript OnTriggerExit with Entity " << other << "\n";
    }

   
    float getCoef() {
      return boostCoef;
    }

    int getDuration() {
      return boostDuration;
    }


  };
}
