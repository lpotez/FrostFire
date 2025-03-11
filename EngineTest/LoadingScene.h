#pragma once
#include "BaseScene.h"
#include "EngineWindows.h"

namespace FrostFireEngine {

  class LoadingScene : public BaseScene
  {
    bool isMainSceneLoading = false;
    bool hasRenderedOnce = false;
    void Initialize(DispositifD3D11 *pDevice) override;

  public:


    void Update(float deltaTime) override;
  };

}
