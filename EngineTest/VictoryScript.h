#pragma once
#include "ECS/components/rendering/PBRRenderer.h"
#include "InputManager.h"
#include <vector>
#include <fstream>


#include "CheckpointScript.h"
#include "ScriptTime.h"
#include "ECS/components/physics/RigidBodyComponent.h"
#include "ECS/components/rendering/UIRendererComponent.h"
#include "ECS/systems/PauseManagerSystem.h"
#include "ECS/systems/PhysicsSystem.h"


namespace FrostFireEngine
{
  class VictoryScript final : public ScriptComponent {
    static const int MAXTURNS = 2;
    static const int MAXPOINTS = MAXTURNS * CheckpointScript::N;
    int              pointsUntilVictory;
    bool             gameOver{ false };
    bool             displayedUI{ false };
    float            victoryTimer{ 0.0f };
    EntityId         gameOverUI;

    RigidBodyComponent *rb;
    UIRendererComponent *victoryUiRenderer = nullptr;

    int *checkpointManager;
    std::vector<EntityId> checkpoints;

    ScriptTime *st;
    ScriptDecompte *sd;

  public:
    VictoryScript(int *p, std::vector<EntityId> list, EntityId id) : checkpointManager{ p },
      checkpoints{ list }, gameOverUI{ id }
    {
    }
    void Awake() override
    {
      rb = GetEntity()->GetComponent<RigidBodyComponent>();
      victoryUiRenderer = World::GetInstance().GetEntity(gameOverUI)->GetComponent<
        UIRendererComponent>();
      pointsUntilVictory = MAXPOINTS;
      st = World::GetInstance().GetEntitiesWith<ScriptTime>()[0]->GetComponent<ScriptTime>();
      sd = World::GetInstance().GetEntitiesWith<ScriptDecompte>()[0]->GetComponent<ScriptDecompte>();
    }

    void Start() override
    {
    }

    void Update(float deltaTime) override
    {
      if (IsGamePaused()) return;
      if (InputManager::GetInstance().IsKeyPressed(DIK_SPACE)) {

        Reset();

      }
      if (!gameOver && pointsUntilVictory == 1) {
        RenderFinalCheckpoint();
      }
      if (!gameOver && pointsUntilVictory == 0) {
        Victory();
      }
      if (gameOver && !displayedUI) {
        victoryTimer += deltaTime;
        if (victoryTimer >= 2.0f) {
          AudioSystem &player = AudioSystem::Get();
          player.PlaySound("Assets/Sounds/Victory.wav", 0.8f, false);
          victoryUiRenderer->SetVisible(true);
          displayedUI = true;
        }
      }
    }

    void OnCollisionEnter(const EntityId other) override
    {
      //std::cout << "MyScript OnCollisionEnter with Entity " << other << "\n";
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
      //std::cout << "MyScript OnTriggerEnter with Entity " << other << "\n";
    }

    void OnTriggerStay(const EntityId other) override
    {
      //std::cout << "MyScript OnTriggerStay with Entity " << other << "\n";
    }

    void OnTriggerExit(const EntityId other) override
    {
      //std::cout << "MyScript OnTriggerExit with Entity " << other << "\n";
    }


    void Reset()
    {

      *checkpointManager = 0;
      gameOver = false;
      displayedUI = false;
      victoryTimer = 0.0f;
      pointsUntilVictory = MAXPOINTS;
      rb->Teleport({ 87.5f, -20.5f, -35.5f }, { 0.f, 0.7071068f, 0.f, 0.7071068f });
      PBRRenderer *rd = World::GetInstance().
        GetEntity(checkpoints[0])->
        GetComponent<PBRRenderer>();
      rd->SetBaseColor({ 0.0f, 0.8f, 1.0f, 0.5f });


      victoryUiRenderer->SetVisible(false);
      st->Reset();
      sd->ResetCountdown();

    }

    void CheckpointReached()
    {
      pointsUntilVictory--;
    }

    void Victory()
    {
      gameOver = true;
      rb->Teleport({ -15.5f, 28.f, 22.0f }, { 0.f, 0.7071068f, 0.f, 0.7071068f });
      st->SetVictory(true);

      auto now = std::chrono::system_clock::now();
      auto in_time_t = std::chrono::system_clock::to_time_t(now);

      std::ostringstream oss;
      std::tm tm;
      localtime_s(&tm, &in_time_t);
      oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

      {
        std::ofstream out{ "scores.txt", std::ios::app };
        if (out)
        {
          out << st->GetTime() << "|" << oss.str() << std::endl;
        }
        
      }
    }

    bool IsValidCheckpoint(int id)
    {
      bool isValid{ id == (*checkpointManager + 1) % CheckpointScript::N };
      if (isValid) {
        *checkpointManager = (*checkpointManager + 1) % CheckpointScript::N;
      }
      return isValid;
    }

    bool IsGameOver()
    {
      return gameOver;
    }

    bool IsGamePaused()
    {
      return PauseManagerSystem::Get().IsPaused();
    }

    void RenderFinalCheckpoint()
    {

      PBRRenderer *rd = World::GetInstance().
        GetEntity(checkpoints[0])->
        GetComponent<PBRRenderer>();
      rd->SetBaseColor({ 0.0f, 1.0f, 0.0f, 0.5f });
    }
  };
}
