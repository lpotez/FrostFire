#pragma once
#include "Singleton.h"
#include "Scene.h"
#include <memory>
#include <functional>

namespace FrostFireEngine
{
  class SceneManager : public CSingleton<SceneManager> {
  public:
    void Update(float dt)
    {
      if (pendingSceneLoader) {
        if (activeScene) {
          Cleanup();
        }

        pendingSceneLoader();
        pendingSceneLoader = nullptr;
      }

      if (activeScene) {
        activeScene->Update(dt);
      }
    }

    Scene* GetActiveScene() const
    {
      return activeScene.get();
    }

    template <typename T, typename... Args>
    void SetActiveScene(DispositifD3D11* pDevice, Args&&... args)
    {
      static_assert(std::is_base_of_v<Scene, T>, "T doit dériver de Scene.");

      if (activeScene) {
        pendingSceneLoader = [this, pDevice, ... args = std::forward<Args>(args)]() mutable
        {
          activeScene = std::make_unique<T>(std::forward<Args>(args)...);
          activeScene->Initialize(pDevice);
        };
      }
      else {
        Cleanup();
        activeScene = std::make_unique<T>(std::forward<Args>(args)...);
        activeScene->Initialize(pDevice);
      }
    }

    void Cleanup()
    {
      activeScene.reset();
    }

  private:
    std::unique_ptr<Scene> activeScene;
    std::function<void()>  pendingSceneLoader;
  };
}
