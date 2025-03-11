#pragma once

#include <memory>
#include <cstdint>
#include "Engine/Singleton.h"
#include "dispositif.h"
#include <d3dx11effect.h>
#include "Engine/ECS/core/World.h"
#include "InputManager.h"
#include "MeshManager.h"
#include "Clock.h"
#include "SceneManager.h"
#include "Core/PhysicsResources.h"
#include "ECS/systems/CameraSystem.h"
#include "ECS/systems/RenderingSystem.h"

namespace FrostFireEngine
{
  class InputSystem;

  constexpr int    IMAGES_PAR_SECONDE = 60;
  constexpr double ECART_TEMPS = 1.0 / static_cast<double>(IMAGES_PAR_SECONDE);


  template <class T, class TDevice>
  class Engine : public CSingleton<T> {
  public:
    void Run()
    {
      bool isRunning = true;

      while (isRunning) {
        isRunning = RunSpecific();

        if (isRunning) {
          isRunning = Animate();
        }
      }
    }

    int Initialize()
    {
      inputManager.Init();
      InitialisationsSpecific();
      pDevice = CreationDispositifSpecific(CDS_FENETRE);

      #if _DEBUG
      InitialisationsImGui();
      #endif

      InitializeAnimation();
      return 0;
    }

    void Cleanup()
    {
      inputManager.Release();

      if (pDevice) {
        delete pDevice;
        pDevice = nullptr;
      }

      SceneManager::GetInstance().Cleanup();
      PhysicsResources::GetInstance().Cleanup();
    }

    TDevice* GetDevice()
    {
      return pDevice;
    }

  protected:
    ~Engine() override = default;

    virtual bool RunSpecific() = 0;
    virtual int  InitialisationsSpecific() = 0;
    #if _DEBUG
    virtual void InitialisationsImGui() = 0;
    #endif
    virtual TDevice* CreationDispositifSpecific(CDS_MODE cdsMode) = 0;
    virtual void     BeginRenderSceneSpecific() = 0;
    virtual void     EndRenderSceneSpecific() = 0;

    int64_t GetTimeSpecific()
    {
      return m_Horloge.GetTimeCount();
    }

    double GetTimeIntervalsInSec(int64_t start, int64_t stop) const
    {
      return m_Horloge.GetTimeBetweenCounts(start, stop);
    }

    int64_t nextTime = 0;
    int64_t previousCounterTime = 0;

    TDevice* pDevice = nullptr;

    InputManager& inputManager = InputManager::GetInstance();
    Clock         m_Horloge;

    int InitializeAnimation()
    {
      previousCounterTime = GetTimeSpecific();
      nextTime = previousCounterTime;

      return 0;
    }

    int timer = 0;

    void RenderScene()
    {
      if (auto* cameraSystem = World::GetInstance().GetSystem<CameraSystem>()) {
        if (auto cameraComponent = cameraSystem->GetActiveCameraComponent()) {
          cameraComponent->UpdateViewMatrix();
          cameraComponent->UpdateOrthoMatrix(pDevice);
        }
        else {
          throw std::runtime_error("Active camera or its components are missing.");
        }
      }
      else {
        throw std::runtime_error("CameraSystem not initialized.");
      }
    }

    bool Animate()
    {
      int64_t currentCounterTime = GetTimeSpecific();
      double  elapsedTime = GetTimeIntervalsInSec(previousCounterTime, currentCounterTime);
      if (elapsedTime > ECART_TEMPS) {
        pDevice->Present();

        AnimateScene(elapsedTime);
        RenderScene();
        previousCounterTime = currentCounterTime;
      }
      return true;
    }

    void AnimateScene(double deltaTime)
    {
      float dt = static_cast<float>(deltaTime);
      inputManager.UpdateKeyboard();
      inputManager.UpdateMouse();
      SceneManager::GetInstance().Update(dt);
    }
  };
} // namespace PM3D
