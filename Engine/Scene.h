#pragma once
#include "DispositifD3D11.h"
#include "Engine/ECS/core/World.h"

namespace FrostFireEngine
{
  class SceneManager;

  class Scene {
  public:
    friend class SceneManager;

    virtual ~Scene();

    World&       GetWorld();
    virtual void Initialize(DispositifD3D11* pDevice) = 0;
    virtual void Update(float deltaTime);

  private:
    World world;

  protected:
    Scene() = default;
  };
}
