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
#include <ECS/components/LightComponent.h>


namespace FrostFireEngine
{
  class AreaSpecificsScript final : public ScriptComponent {



    EntityId lightId;
    LightComponent* light;

  
    bool handlerOver = true;
    bool firstCheckpoint;

    float initialIntensity;
    float deltaIntensity = 1.0;

    int time = 0;
    float duration = 0;

  public:

    const float MaxI = 2.0;

    const float MinI = 0.2;

    enum Specifics {
      LIGHTER,
      DARKER,
      SLIPPERY,
      NORMAL
    };
  private :
    Specifics spec;

  public :
    AreaSpecificsScript(EntityId id, Specifics spec_, bool fst_ = false) : lightId (id), spec(spec_), firstCheckpoint(fst_)
    {
      
      switch (spec_) {
      case LIGHTER :
        deltaIntensity = MinI - MaxI;
        initialIntensity = MinI;
        break;
      case DARKER :
        deltaIntensity = MaxI - MinI;
        initialIntensity = MaxI;
        break;
      default :
        deltaIntensity = 0;
        initialIntensity = 0;
      }
      
    }
    void Awake() override
    {
      auto owner = World::GetInstance().GetEntity(lightId);
      light = owner->GetComponent<LightComponent>();
    
    }

    void Start() override
    {
    }

    void Update(float deltaTime) override
    {
      switch (spec) {
      case DARKER :
        if (light->GetIntensity() > (initialIntensity - deltaIntensity) ) {
          HandleLight();
        }
        else {
          handlerOver = true;
        }
        break;
      case LIGHTER :
        if (light->GetIntensity() < (initialIntensity - deltaIntensity) ) {
          HandleLight();
        }
        else {
          handlerOver = true;
        }
          break;

      
      
      }
     
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
      int i = 0;
      AcitvateHandler(120);
      if (firstCheckpoint) {
        light->SetIntensity(MaxI);
      }
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


    void AcitvateHandler(int duration_) {
      if (handlerOver) {
        duration = duration_;
        time = 0;
       
        handlerOver = false;
      
      }
    }

    void HandleLight() {
 
      if (!handlerOver) {
        light->SetIntensity(light->GetIntensity() - (1 / duration) * deltaIntensity);
        time++;
      }
      if (time == duration) {
       
        handlerOver = true;
        time = 0;
        duration = 0;
      }
    }


  };
}
