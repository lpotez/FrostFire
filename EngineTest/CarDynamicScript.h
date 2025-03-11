#pragma once
#include <chrono>

#include "VictoryScript.h"

#include "InputManager.h"
#include "ECS/components/physics/RigidBodyComponent.h"
#include "ECS/components/transform/TransformComponent.h"
#include "Engine/ECS/core/World.h"
#include "BoostScript.h"
#include "DeadZoneScript.h"
#include "ScriptTime.h"
#include <AudioSystem.h>

namespace FrostFireEngine
{
  class CarDynamicScript final : public ScriptComponent {
    RigidBodyComponent* rb;
    TransformComponent* tc;
    VictoryScript*      vs;
    ScriptDecompte      *sd;
    ScriptTime *st;
    XMFLOAT3            spawnPosition;
    XMFLOAT4            spawnRotation;
    XMFLOAT3            initialPosition;
    XMFLOAT4            initialRotation;
    UINT                LastKeyPres;




    

    //Coefficient des sigmoïdes pour le calcul des vitesses linéaires et angulaires 
    float k = 100;
    float ka = 2;

    //Coefficients pour déterminer la force à appliquer pour faire avancer la voiture
    float maxCoef = 15.0;
    float minCoef = 10.0;
    float               forceCoef = 0.0f;

    //Paramètres sur les vitesses angulaires et linéaires
    float maxSpeed = 24.0;
    float maxBoostSpeed = 32.0f;
    float maxAngle = XM_PIDIV2;
    float minAngle = XM_PI / 6;

    //Coefficients pour déterminer la vitesse angulaire
    float rotationFactor = 0.0f;
    float angle = 0.0;

   //Coefficients de friction 
    float lateral_coef = 2.0;
    float backward_coef = 0.10;

    bool onGround = false;

    //Gestion de la touche Z : ne s'active que si on touche ->  ou <- mais on peut garder appuyer ensuite
    bool lastImputZ = false;
    bool rightZ_enabled = false;
    bool leftZ_enabled = false;


 
    //Gestion du boost
    int   boostDuration = 0;
    float  boostIncr = 0;
    float boostFactor = 1.0;
    float boostMult = 8.0;





  public:
    void Awake() override
    {
      rb = GetEntity()->GetComponent<RigidBodyComponent>();
      tc = GetEntity()->GetComponent<TransformComponent>();
      vs = GetEntity()->GetComponent<VictoryScript>();
      st = World::GetInstance().GetEntitiesWith<ScriptTime>()[0]->GetComponent<ScriptTime>();
      sd = World::GetInstance().GetEntitiesWith<ScriptDecompte>()[0]->GetComponent<ScriptDecompte>();
      spawnPosition = {87.5f, -20.5f, -35.5f};
      spawnRotation = {0.f, 0.7071068f, 0.f, 0.7071068f};
      initialPosition = {87.5f, -20.5f, -35.5f};
      initialRotation = {0.f, 0.7071068f, 0.f, 0.7071068f};


    }

    void Start() override
    {
    }

    void Update(float deltaTime) override
    {
      if (vs->IsGameOver() || vs->IsGamePaused() || sd->isDecompteActive()) return;


      //Avancer
      if (onGround && InputManager::GetInstance().IsKeyPressed(DIK_UP)) {

        static auto lastExecution = std::chrono::steady_clock::now();

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastExecution);

        if (elapsed.count() >= 400) {
          lastExecution = now;

          AudioSystem &player = AudioSystem::Get();

          player.PlaySound("Assets/Sounds/Voiture1.wav", 0.05f, false);
        }
        auto truc = rb->GetLinearVelocity();
        auto norm = XMVectorGetX(XMVector3Length({truc.x, truc.y, truc.z}));
        forceCoef = maxCoef + (minCoef - maxCoef) /
        (1.0f + exp(-k * (norm - maxSpeed / 2)));
        XMFLOAT3 force;
        XMVECTOR forward = forceCoef * tc->GetForward();
        XMStoreFloat3(&force, forward);
        rb->ApplyForce(force);


      }

      //Reculer
      if ( onGround && InputManager::GetInstance().IsKeyPressed(DIK_DOWN)) {
       
        XMFLOAT3 force;
        XMVECTOR forward = - 9 * tc->GetForward();
        XMStoreFloat3(&force, forward);
        rb->ApplyForce(force);
      }



      //Effet des forces de frictions sur la vitesse
      XMFLOAT3 speed = rb->GetLinearVelocity();
      XMVECTOR vel = {speed.x, speed.y, speed.z};
      float    norm = sqrt(speed.x * speed.x + speed.y * speed.y + speed.z * speed.z);
      bool     isMoving = norm > 0.1f;


      if (isMoving) {
        XMVECTOR lateral_velocity = tc->GetRight() * XMVector3Dot(vel, tc->GetRight());
        XMVECTOR lateral_friction = -lateral_velocity * lateral_coef;
        XMVECTOR backwards_friction = -vel * backward_coef;
        vel += (backwards_friction + lateral_friction) * deltaTime;
      }

      XMStoreFloat3(&speed, vel);

      //Savoir si la voiture avance ou recule
      float dot = XMVectorGetX(XMVector3Dot(tc->GetForward(), vel));



      bool Z_imput = false;

      //Appuyer sur Z : debut du derapage
      if (InputManager::GetInstance().IsKeyPressed(DIK_Z)) {
        Z_imput = true;
       
      }


      //Tourner à gauche
      if (onGround && isMoving && InputManager::GetInstance().
        IsKeyPressed(DIK_LEFT)) {


        if (Z_imput && !rightZ_enabled) {
          leftZ_enabled = true;
        }

        if (leftZ_enabled && dot > 0) {
          Z_imput = true;
          lastImputZ = true;
          auto speed = rb->GetLinearVelocity();
          auto norm = XMVectorGetX(XMVector3Length({ speed.x,speed.y, speed.z }));
          if (norm > 1) {


            float maxAngleS = 1 <= norm <= 2 ? XM_PIDIV2 : 3 * XM_PI / 8;

            rotationFactor += deltaTime / 2;
            angle = minAngle + (maxAngleS - minAngle) /
            (1.0f + exp(-ka * (rotationFactor - 0.5)));

          }
        }
        else {
          angle = maxAngle / 2 + (minAngle - maxAngle / 2) /
          (1.0f + exp(-ka * (norm - maxSpeed / 2)));
        }


        if (dot > 0.01f) {
          rb->SetAngularVelocity({0.0, -angle, 0.0});
        }
        else if (dot < -0.1f) {
          rb->SetAngularVelocity({0.0, angle, 0.0});
        }
        
      }


      //Tourner a droite
      if (onGround && isMoving && InputManager::GetInstance().IsKeyPressed(DIK_RIGHT)) {


        if (Z_imput && !leftZ_enabled) {
          rightZ_enabled = true;
        }

        if (rightZ_enabled && dot > 0) {
          Z_imput = true;
          lastImputZ = true;
          auto speed= rb->GetLinearVelocity();
          auto norm = XMVectorGetX(XMVector3Length({ speed.x,speed.y, speed.z }));
          if (norm > 1) {

            float maxAngleS = 1 <= norm <= 2 ? XM_PIDIV2 : 3 * XM_PI / 8;

            rotationFactor += deltaTime / 2;
            angle = minAngle + (maxAngleS - minAngle) /
            (1.0f + exp(-ka * (rotationFactor - 0.5)));


          }
        }
        else {
          angle = maxAngle / 2 + (minAngle - maxAngle / 2) /
          (1.0f + exp(-ka * (norm - maxSpeed / 2)));
        }


        if (dot < -0.01f) {
          rb->SetAngularVelocity({0.0, -angle, 0.0});
        }
        else if (dot > 0.1f) {
          rb->SetAngularVelocity({0.0, angle, 0.0});
        }
        
      }


      rb->SetLinearVelocity(speed);

      //Fin du dérapage
      if (!Z_imput) {
        maxCoef = 15.0;
        minCoef = 11.0;
        if (lastImputZ) {
          SetBoost(rotationFactor <= 0.4 ? 1.0 : (rotationFactor <= 0.8 ? 1.2 : 1.5),
                   rotationFactor <= 0.4 ? 0.0 : (rotationFactor <= 0.8 ? 60 : 120));


        }
        rotationFactor = 0.0;
        lastImputZ = false;
        leftZ_enabled = false;
        rightZ_enabled = false;
       
      }
      rotationFactor = rotationFactor > 1.0 ? 1.0 : (rotationFactor < 0.0 ? 0.0 : rotationFactor);

      //Gestion du boost
      ApplyBoost();

      //Revenir au dernier point d'apparition
      if (InputManager::GetInstance().IsKeyPressed(DIK_C)) {
        Respawn();
      }

      //Relancer une partie
      if (InputManager::GetInstance().IsKeyPressed(DIK_SPACE)) {
        ResetPositionAndRotation();
      }

      //Redresser la voiture
      if (InputManager::GetInstance().IsKeyPressed(DIK_X)) {
        Redress();
      
      }
    }


    void OnCollisionEnter(const EntityId other) override
    {

      //Son de collision

      static auto lastExecution = std::chrono::steady_clock::now();

      auto now = std::chrono::steady_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastExecution);

      rotationFactor *= 0.5;
      if (auto _other = World::GetInstance().GetEntity(other)->GetComponent<DeadZoneScript>()) {
        lastExecution = now;
        AudioSystem &player = AudioSystem::Get();
        player.PlaySound("Assets/Sounds/Death.wav", 0.8f, false);
        Respawn();
        return;
      }


      if (elapsed.count() >= 3000) {
        lastExecution = now;

        AudioSystem& player = AudioSystem::Get();

        player.PlaySound("Assets/Sounds/Bonk.wav", 0.8f, false);
      }
    }

    void OnCollisionStay(const EntityId other) override
    {
      onGround = true;
    }

    void OnCollisionExit(const EntityId other) override
    {
      onGround = false;
    }

    void OnTriggerEnter(const EntityId other) override
    {
      auto              _other = World::GetInstance().GetEntity(other);
      CheckpointScript* cs = _other->GetComponent<CheckpointScript>();

      //Gestion du checkpoint
      if (cs) {
        if (vs->IsValidCheckpoint(_other->GetComponent<CheckpointScript>()->GetId())) {
          spawnPosition = _other->GetComponent<CheckpointScript>()->GetPosition();
          spawnRotation = _other->GetComponent<CheckpointScript>()->GetRotation();
          _other->GetComponent<BaseRendererComponent>()->SetVisible(false);

          vs->CheckpointReached();
        }
        return;

      }

      //Initier un boost
      BoostScript* bs = _other->GetComponent<BoostScript>();
      if (bs) {
        AudioSystem &player = AudioSystem::Get();
        player.PlaySound("Assets/Sounds/Grandboost.wav", 0.8f, false);

        SetBoost(bs->getCoef(), bs->getDuration());
        return;
      }

      //Gestion du hors-pisye
      DeadZoneScript* dzs = _other->GetComponent<DeadZoneScript>();
      if (dzs) {
        AudioSystem &player = AudioSystem::Get();
        player.PlaySound("Assets/Sounds/Death.wav", 0.8f, false);

        Respawn();
      }
    
    }

    void OnTriggerStay(const EntityId other) override
    {
    }

    void OnTriggerExit(const EntityId other) override
    {
    }

    void ResetPositionAndRotation()
    {
      resetBoost();
      rotationFactor = 0.0;
      spawnPosition = initialPosition;
      spawnRotation = initialRotation;
      st->Reset();
      sd->ResetCountdown();
     
    }

    
    void Respawn()
    {
      resetBoost();
      rotationFactor = 0.0;
      rb->Teleport(spawnPosition, spawnRotation);
    }

    void Redress()
    {
      resetBoost();
      rotationFactor = 0.0;
      tc->SetRotationEuler({0, tc->GetRotationEuler().y, 0});
      rb->Teleport(tc->GetPosition(), tc->GetRotation());
    }

    void SetBoost(float boostFactor_, int boostDuration_)
    {

      boostFactor = boostFactor_;
      boostDuration = boostDuration_;
      boostIncr = boostDuration_;

    }
    void ApplyBoost()
    {
      if (boostDuration == 0) return;
      auto vel = rb->GetLinearVelocity();
     

      auto     fwdVec = tc->GetForward();
      XMFLOAT3 fwd;
      float    val = boostFactor / boostDuration;
      fwdVec = XMVector3Normalize(fwdVec);
      XMStoreFloat3(&fwd, fwdVec);

      //On applique le boost sur une frame sinon ce n'est pas vraiment un boost
      if (boostIncr == boostDuration) {

        rb->SetLinearVelocity({ boostMult * boostFactor * fwd.x + vel.x,  boostMult * boostFactor * fwd.y + vel.y,
           boostMult * boostFactor * fwd.z + vel.z});

        AudioSystem &player = AudioSystem::Get();
        player.PlaySound("Assets/Sounds/Petitboost.wav", 0.8f, false);

      }

      boostIncr--;


      //Si le boost fait arriver à une vitesse supérieure à la vitesse max
      if (auto norm = sqrt(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z); 0 < boostIncr &&
        boostIncr <= (boostDuration / 4) && norm >= maxSpeed) {

        float delta = norm - maxSpeed;
        float fact = delta * (1 / boostIncr);
        rb->SetLinearVelocity({-fact * fwd.x + vel.x, -fact * fwd.y + vel.y,
          -fact * fwd.z + vel.z});
      }

    }

    void resetBoost()
    {
      boostIncr = 0;
      boostDuration = 0;
      boostFactor = 1.0;
    }

    float GetMaxBoostSpeed() {
      return maxBoostSpeed;
    }
  };
}
