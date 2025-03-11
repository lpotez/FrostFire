#pragma once

#include <iostream>
#include <sstream>

#include "ECS/components/physics/RigidBodyComponent.h"
#include "ECS/components/rendering/TextRendererComponent.h"
#include "ECS/components/scripts/ScriptComponent.h"
#include "ECS/systems/PauseManagerSystem.h"
#include "ScriptPauseManager.h"

namespace FrostFireEngine
{
  class ScriptDecompte final : public ScriptComponent
  {
  public:

    ScriptDecompte(const EntityId &vehiculeId) : vehiculeId_(vehiculeId), audioSystem(AudioSystem::Get())
    {
      rbVehicule_ = World::GetInstance().GetEntity(vehiculeId_)->GetComponent<RigidBodyComponent>();
    };

    void Start() override
    {
      textRenderer = GetEntity()->GetComponent<TextRendererComponent>();
      PauseManagerSystem::Get().SetPaused(true);
      rbVehicule_->SetEnabled(false);
      timer = cooldown;
      sp = World::GetInstance().GetEntitiesWith<ScriptPauseManager>()[0]->GetComponent<ScriptPauseManager>();
      isCountdownFinished = false;
    }

    void Update(float deltaTime) override
    {
      if (sp->GetCurrentState())
      {
        audioSystem.PauseAllSounds();
        textRenderer->SetVisible(false);
        return;
      }
      textRenderer->SetVisible(true);
      audioSystem.ResumeAllSounds();

      if (isCountdownFinished) {
        timer = cooldown;
        textRenderer->SetVisible(false);
        return;
      }

      timer -= deltaTime;

      if (timer <= 0.0f)
      {
        if (decompte == 4)
        {
          audioSystem.PlaySound("Assets/Sounds/decompte.wav", 0.5f, false);
        }
        --decompte;
        std::wstringstream wss;
        
        if (decompte > 0)
        {
          wss << decompte;
        }
        else
        {
          wss << L"PARTEZ!";
          PauseManagerSystem::Get().SetPaused(false);
          isCountdownFinished = true;

          if (rbVehicule_)
            rbVehicule_->SetEnabled(true);
        }
        textRenderer->SetText(wss.str());

        GetEntity()->GetComponent<RectTransformComponent>()->SetSize({ textRenderer->GetContentWidth(), textRenderer->GetContentHeight() });
        timer = cooldown;

      }
    }

    bool isDecompteActive()
    {
      return !isCountdownFinished;
    }

    void ResetCountdown()
    {
      decompte = 4;
      timer = 0.0f;
      isCountdownFinished = false;
    }

  private:
    int decompte = 4;
    float timer = 0.0f;
    float cooldown = 1.0;
    bool isCountdownFinished = false;

    EntityId vehiculeId_;
    RigidBodyComponent *rbVehicule_ = nullptr;
    TextRendererComponent *textRenderer = nullptr;
    AudioSystem &audioSystem;
    ScriptPauseManager *sp = nullptr;
  };
}

