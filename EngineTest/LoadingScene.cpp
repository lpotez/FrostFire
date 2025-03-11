#include "LoadingScene.h"
#include "MainScene.h"
#include "SceneManager.h"
#include "TextureManager.h"

namespace FrostFireEngine
{
  void LoadingScene::Initialize(DispositifD3D11* pDevice)
  {
    BaseScene::Initialize(pDevice);

    World&          world = GetWorld();
    TextureManager& textureManager = TextureManager::GetInstance();
    InputManager::GetInstance().SetUIMode(true);

    {
      auto  loadingEntity = world.CreateEntity();
      auto& transform = loadingEntity->AddComponent<TransformComponent>();
      auto& rectTransform = loadingEntity->AddComponent<RectTransformComponent>(pDevice);
      auto& renderer = loadingEntity->AddComponent<UIRendererComponent>(pDevice);
      renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/loadingScreen.png", pDevice));

      rectTransform.SetAnchor(RectAnchorPreset::MiddleCenter);
      rectTransform.SetSize({pDevice->GetViewportWidth(), pDevice->GetViewportHeight()});
      renderer.SetVisible(true);
    }

    isMainSceneLoading = false;
    hasRenderedOnce = false;
  }

  void LoadingScene::Update(float deltaTime)
  {
    BaseScene::Update(deltaTime);
    if (!hasRenderedOnce) {
      hasRenderedOnce = true;
      return;
    }

    if (!isMainSceneLoading) {
      isMainSceneLoading = true;
      auto& sceneManager = SceneManager::GetInstance();
      sceneManager.SetActiveScene<MainScene>(EngineWindows::GetInstance().GetDevice());
    }
  }
}
