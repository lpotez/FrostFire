#include "ScriptComponent.h"
#include <iostream>

#include "Engine/ECS/systems/ScriptSystem.h"

namespace FrostFireEngine
{
  void ScriptComponent::OnAttach()
  {
    RegisterWithScriptSystem();
  }

  void ScriptComponent::OnDetach()
  {
    UnregisterFromScriptSystem();
  }

  void ScriptComponent::RegisterWithScriptSystem()
  {
    if (const auto system = World::GetInstance().GetSystem<ScriptSystem>())
    {
      scriptSystem = system;
      scriptSystem->RegisterScript(this);
    }
    else
    {
      std::cerr << "ScriptSystem not found!\n";
    }
  }

  void ScriptComponent::UnregisterFromScriptSystem()
  {
    if (!owner || !scriptSystem) return;

    scriptSystem->UnregisterScript(this);
    scriptSystem = nullptr;
  }

  void ScriptComponent::SetEnabled(bool value) noexcept
  {
    if (enabled != value)
    {
      Component::SetEnabled(value);
      if (scriptSystem)
      {
        if (enabled)
        {
          scriptSystem->OnScriptEnabled(this);
        }
        else
        {
          scriptSystem->OnScriptDisabled(this);
        }
      }
    }
  }
}
