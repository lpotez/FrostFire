#pragma once
#include "ECS/components/rendering/PBRRenderer.h"
#include "ECS/components/scripts/ScriptComponent.h"
#include "ECS/components/transform/TransformComponent.h"


namespace FrostFireEngine
{
  class CheckpointScript final : public ScriptComponent {
    TransformComponent* transform = nullptr;
    int                 checkpointId;
    int*                currentId;
    PBRRenderer*        rc;

    XMFLOAT3 spawnPosition = {-1000,-1000,-1000};
    XMFLOAT4 spawnRotation = { -1000,-1000,-1000, -1000 };;
    //VictoryScript *vs;

    //int currentId{ 0 };

  public:
    static const int N = 4;

    CheckpointScript(int id, int* p) : checkpointId{id}, currentId{p}
    {

    }

    void Awake() override
    {


      rc = GetEntity()->GetComponent<PBRRenderer>();
      transform = GetEntity()->GetComponent<TransformComponent>();
      if (transform) {
        if (spawnPosition.x == -1000 && spawnPosition.y == -1000 && spawnPosition.z == -1000) {
          spawnPosition = transform->GetPosition();
         }
        if (spawnRotation.x == -1000 && spawnRotation.y == -1000 && spawnRotation.z == -1000 && spawnRotation.w == -1000) {
          spawnRotation = transform->GetRotation();
        }
      }

    }

    void Start() override
    {

    }

    void Update(float /*deltaTime*/) override
    {
      if (*currentId == (checkpointId + N - 1) % N) {
        rc->SetVisible(true);


      }
      else {
        rc->SetVisible(false);
      }
    }

    void OnCollisionEnter(const EntityId other) override
    {

    }

    void OnCollisionStay(const EntityId other) override
    {

    }

    void OnCollisionExit(const EntityId other) override
    {

    }

    void OnTriggerEnter(const EntityId other) override
    {

    }

    void OnTriggerStay(const EntityId other) override
    {

    }

    void OnTriggerExit(const EntityId other) override
    {

    }

    int GetId()
    {
      return checkpointId;
    }

    void renderFinalCheckPoint()
    {

      rc->SetBaseColor({0.65f, 0.85f, 0.0f, 1.0f});

    }

    void SetPosition(XMFLOAT3 pos) {
      spawnPosition = pos;
    }

    void SetRotation(XMFLOAT4 rot) {
      spawnRotation = rot;
    }

    XMFLOAT3 GetPosition() {
      return spawnPosition;
    }
    XMFLOAT4 GetRotation() {
      return spawnRotation;
    }

  };
}
