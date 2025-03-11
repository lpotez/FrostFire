#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>

#include "ScriptDecompte.h"

namespace FrostFireEngine
{
  class ScriptTime final : public ScriptComponent
  {
  public:

    ScriptTime() = default;

    void Start() override
    {
      sd = World::GetInstance().GetEntitiesWith<ScriptDecompte>()[0]->GetComponent<ScriptDecompte>();
    }

    void Update(float deltaTime) override
    {
      std::wstringstream wss;
      wss << std::fixed << std::setprecision(3) << tempsEcoule;
      if (PauseManagerSystem::Get().IsPaused() || isVictory || sd->isDecompteActive()) {
        GetEntity()->GetComponent<TextRendererComponent>()->SetText(wss.str());
      }
      else {
        tempsEcoule += deltaTime;
        GetEntity()->GetComponent<TextRendererComponent>()->SetText(wss.str());
      }
    }

    void Reset()
    {
      tempsEcoule = 0.0f;
      SetVictory(false);
    }

    void SetVictory(bool victory)
    {
      isVictory = victory;
    }

    float GetTime() const
    {
      return tempsEcoule;
    }

  private:
    float tempsEcoule = 0.0f;
    bool isVictory = false;
    ScriptDecompte *sd;
  };
}
