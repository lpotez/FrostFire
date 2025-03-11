#pragma once
#include "BaseScene.h"
#include "LoadingScene.h"
#include "ECS/components/SliderComponent.h"
#include "ECS/components/rendering/TextRendererComponent.h"
#include "Font/FontManager.h"
#include "TextureManager.h"
#include "ECS/components/ButtonSoundComponent.h"

namespace FrostFireEngine
{
  class MenuScene : public BaseScene {

  private:
    bool isMainMenuActive = true;
    bool isSettingsMenuActive = false;
    bool isLeaderboardActive = false;

    std::vector<std::shared_ptr<Entity>> mainMenuEntities;
    std::vector<std::shared_ptr<Entity>> settingsMenuEntities;
    std::vector<std::shared_ptr<Entity>> leaderboardEntities;

    float padding = 40.0f;
    float logoHeight = 250.0f;

  public:
    void Initialize(DispositifD3D11* pDevice) override
    {
      BaseScene::Initialize(pDevice);

      AudioSystem &player = AudioSystem::Get();
      player.PlaySound("Assets/Sounds/SonMenu.wav", 0.04f, true);

      World& world = GetWorld();

      TextureManager& textureManager = TextureManager::GetInstance();
      FontManager& fontManager = FontManager::GetInstance();
      Font* font = fontManager.LoadFont(L"Assets/Fonts/font.txt", L"Assets/Fonts/font.png",
                                        pDevice);
      InputManager::GetInstance().SetUIMode(true);
      AudioSystem& AudioSystem = AudioSystem::Get();

      // ========================================= MAIN MENU =========================================

      // LOGO FROSTFIRE
      auto logoEntity = world.CreateEntity();
      {
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        auto &rectTransform = logoEntity->AddComponent<RectTransformComponent>(pDevice);
        auto &renderer = logoEntity->AddComponent<UIRendererComponent>(pDevice);
        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/logo_frostfire.png", pDevice));

        rectTransform.SetAnchor(RectAnchorPreset::TopCenter);
        rectTransform.SetAnchorOffset(XMFLOAT2(0.0f, padding)); 
        rectTransform.SetSize({ logoHeight, logoHeight }); 

        mainMenuEntities.push_back(logoEntity);
      }

      // PLAY BUTTON
      auto buttonEntityPlay = world.CreateEntity();
      {
        auto &transform = buttonEntityPlay->AddComponent<TransformComponent>();
        auto& rectTransform = buttonEntityPlay->AddComponent<RectTransformComponent>(pDevice);
        auto& renderer = buttonEntityPlay->AddComponent<UIRendererComponent>(pDevice);
        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/boutonCraquele.jpg", pDevice));
        auto& button = buttonEntityPlay->AddComponent<ButtonComponent>(pDevice);

        rectTransform.SetAnchor(RectAnchorPreset::TopCenter);
        rectTransform.SetAnchorOffset(XMFLOAT2(0.0f, logoHeight + 2 * padding)); 
        rectTransform.SetSize({250.0f, 50.0f});

        ButtonComponent::ButtonColors colors;
        colors.normal = {0.2f, 0.6f, 1.0f, 1.0f}; // Blue
        colors.hovered = {0.3f, 0.7f, 1.0f, 1.0f}; // Lighter blue
        colors.pressed = {0.1f, 0.5f, 0.9f, 1.0f}; // Darker blue
        button.SetColors(colors);

        button.OnClick([]
        {
          InputManager::GetInstance().SetUIMode(false);

          auto& sceneManager = SceneManager::GetInstance();
          auto loadingScene = std::make_shared<LoadingScene>();
          sceneManager.SetActiveScene<LoadingScene>(EngineWindows::GetInstance().GetDevice());
        });
      }

      auto entityPlayText = world.CreateEntity();
      {
        auto& transform = entityPlayText->AddComponent<TransformComponent>();
        auto& rectTransform = entityPlayText->AddComponent<RectTransformComponent>(pDevice);
        auto& textRenderer = entityPlayText->AddComponent<TextRendererComponent>(pDevice);

        transform.SetParent(buttonEntityPlay->GetId());

        textRenderer.SetFont(font);
        textRenderer.SetText(L"JOUER");
        auto size = textRenderer.SetFontSize(50.0f);
        rectTransform.SetSize(size);
        textRenderer.SetColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

        rectTransform.SetAnchor(RectAnchorPreset::MiddleCenter);
      }

      mainMenuEntities.push_back(buttonEntityPlay);
      mainMenuEntities.push_back(entityPlayText);

      // EXIT BUTTON
      auto buttonEntityExit = world.CreateEntity();
      {
        auto &transform = buttonEntityExit->AddComponent<TransformComponent>();
        auto& rectTransform = buttonEntityExit->AddComponent<RectTransformComponent>(pDevice);
        auto& renderer = buttonEntityExit->AddComponent<UIRendererComponent>(pDevice);
        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/boutonCraquele.jpg", pDevice));
        auto& button = buttonEntityExit->AddComponent<ButtonComponent>(pDevice);

        rectTransform.SetSize({ 250.0f, 50.0f }); 
        rectTransform.SetAnchor(RectAnchorPreset::TopCenter);
        rectTransform.SetAnchorOffset(XMFLOAT2(0.0f, logoHeight + 8 * padding));
        
        ButtonComponent::ButtonColors colors;
        colors.normal = { 0.2f, 0.6f, 1.0f, 1.0f }; // Blue
        colors.hovered = {0.3f, 0.7f, 1.0f, 1.0f}; // Lighter blue
        colors.pressed = {0.1f, 0.5f, 0.9f, 1.0f}; // Darker blue
        button.SetColors(colors);


        button.OnClick([]
        {
          DestroyWindow(GetActiveWindow());
        });
      }

      auto entityExitText = world.CreateEntity();
      {

        auto& transform = entityExitText->AddComponent<TransformComponent>();
        auto& rectTransform = entityExitText->AddComponent<RectTransformComponent>(pDevice);
        auto& textRenderer = entityExitText->AddComponent<TextRendererComponent>(pDevice);

        transform.SetParent(buttonEntityExit->GetId());

        textRenderer.SetFont(font);
        textRenderer.SetText(L"QUITTER");
        auto size = textRenderer.SetFontSize(50.0f);
        rectTransform.SetSize(size);
        rectTransform.SetAnchor(RectAnchorPreset::MiddleCenter);
        textRenderer.SetColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
      }

      mainMenuEntities.push_back(buttonEntityExit);
      mainMenuEntities.push_back(entityExitText);

      // SETTINGS BUTTON
      auto buttonEntitySettings = world.CreateEntity();
      {
        auto &transform = buttonEntitySettings->AddComponent<TransformComponent>();
        auto& rectTransform = buttonEntitySettings->AddComponent<RectTransformComponent>(pDevice);
        auto& renderer = buttonEntitySettings->AddComponent<UIRendererComponent>(pDevice);
        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/boutonCraquele.jpg", pDevice));
        auto& button = buttonEntitySettings->AddComponent<ButtonComponent>(pDevice);

        rectTransform.SetAnchor(RectAnchorPreset::TopCenter);
        rectTransform.SetAnchorOffset({0.0f, logoHeight + 4 * padding });
        rectTransform.SetSize({250.0f, 50.0f});

        ButtonComponent::ButtonColors colors;
        colors.normal = { 0.2f, 0.6f, 1.0f, 1.0f }; // Blue
        colors.hovered = {0.3f, 0.7f, 1.0f, 1.0f}; // Lighter blue
        colors.pressed = {0.1f, 0.5f, 0.9f, 1.0f}; // Darker blue
        button.SetColors(colors);

        button.OnClick([this]
        {
          ShowSettingsMenu();
        });
      }

      auto entitySettingsText = world.CreateEntity();
      {
        auto& transform = entitySettingsText->AddComponent<TransformComponent>();
        auto& rectTransform = entitySettingsText->AddComponent<RectTransformComponent>(pDevice);
        auto& textRenderer = entitySettingsText->AddComponent<TextRendererComponent>(pDevice);

        transform.SetParent(buttonEntitySettings->GetId());

        textRenderer.SetFont(font);
        textRenderer.SetText(L"PARAMETRES");
        auto size = textRenderer.SetFontSize(50.0f);
        rectTransform.SetSize(size);
        rectTransform.SetAnchor(RectAnchorPreset::MiddleCenter);
        textRenderer.SetColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
      }

      mainMenuEntities.push_back(buttonEntitySettings);
      mainMenuEntities.push_back(entitySettingsText);

      // LEADERBOARD BUTTON
      auto buttonEntityLeaderboard = world.CreateEntity();
      {
        auto &transform = buttonEntityLeaderboard->AddComponent<TransformComponent>();
        auto &rectTransform = buttonEntityLeaderboard->AddComponent<RectTransformComponent>(pDevice);
        auto &renderer = buttonEntityLeaderboard->AddComponent<UIRendererComponent>(pDevice);
        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/boutonCraquele.jpg", pDevice));
        auto &button = buttonEntityLeaderboard->AddComponent<ButtonComponent>(pDevice);

        rectTransform.SetAnchor(RectAnchorPreset::TopCenter);
        rectTransform.SetAnchorOffset({ 0.0f, logoHeight + 6 * padding });
        rectTransform.SetSize({ 250.0f, 50.0f });

        ButtonComponent::ButtonColors colors;
        colors.normal = { 0.2f, 0.6f, 1.0f, 1.0f }; // Blue
        colors.hovered = { 0.3f, 0.7f, 1.0f, 1.0f }; // Lighter blue
        colors.pressed = { 0.1f, 0.5f, 0.9f, 1.0f }; // Darker blue
        button.SetColors(colors);

        button.OnClick([this]
          {
            ShowLeaderboard();
          });
      }

      auto entityLeaderboardText = world.CreateEntity();
      {
        auto &transform = entityLeaderboardText->AddComponent<TransformComponent>();
        auto &rectTransform = entityLeaderboardText->AddComponent<RectTransformComponent>(pDevice);
        auto &textRenderer = entityLeaderboardText->AddComponent<TextRendererComponent>(pDevice);

        transform.SetParent(buttonEntityLeaderboard->GetId());

        textRenderer.SetFont(font);
        textRenderer.SetText(L"SCORES");
        auto size = textRenderer.SetFontSize(50.0f);
        rectTransform.SetSize(size);
        rectTransform.SetAnchor(RectAnchorPreset::MiddleCenter);
        textRenderer.SetColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
      }

      mainMenuEntities.push_back(buttonEntityLeaderboard);
      mainMenuEntities.push_back(entityLeaderboardText);

      // ========================================= SETTINGS MENU =========================================
      auto settingsEntity = world.CreateEntity();
      {
        auto &settingsTransform = settingsEntity->AddComponent<TransformComponent>();
        auto &settingsRectTransform = settingsEntity->AddComponent<RectTransformComponent>(pDevice);
        auto &settingsRenderer = settingsEntity->AddComponent<UIRendererComponent>(pDevice);
        settingsRenderer.SetTexture(
          textureManager.GetNewTexture(L"Assets/UI/bois.jpg", pDevice));
        settingsRenderer.SetVisible(isSettingsMenuActive);
        settingsRenderer.SetColor({1.0f, 1.0f, 1.0f, 0.75f});

        settingsRectTransform.SetAnchor(RectAnchorPreset::MiddleCenter);
        settingsRectTransform.SetSize({800.0f, 500.0f});

      }

      settingsMenuEntities.push_back(settingsEntity);

      auto buttonCloseEntity = world.CreateEntity();
      {
        auto& settingsTransform = buttonCloseEntity->AddComponent<TransformComponent>();
        auto& settingsRectTransform = buttonCloseEntity->AddComponent<
          RectTransformComponent>(pDevice);
        auto& settingsRenderer = buttonCloseEntity->AddComponent<UIRendererComponent>(pDevice);
        settingsRenderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/cross.png", pDevice));
        auto& button = buttonCloseEntity->AddComponent<ButtonComponent>(pDevice);

        settingsTransform.SetParent(settingsEntity->GetId());

        settingsRenderer.SetVisible(isSettingsMenuActive);

        settingsRectTransform.SetAnchor(RectAnchorPreset::TopRight);
        settingsRectTransform.SetSize({50.0f, 50.0f});

        ButtonComponent::ButtonColors colors;
        //colors.normal = { 0.2f, 0.0f, 1.0f, 1.0f }; // Blue
        colors.hovered = {0.3f, 0.7f, 1.0f, 1.0f}; // Lighter blue
        colors.pressed = {0.1f, 0.5f, 0.9f, 1.0f}; // Darker blue
        button.SetColors(colors);

        button.OnClick([this]
        {
          ShowMainMenu();
        });
      }

      settingsMenuEntities.push_back(buttonCloseEntity);

      auto settingsSoundEntity = world.CreateEntity();
      {
        auto& settingsTransform = settingsSoundEntity->AddComponent<TransformComponent>();
        auto& settingsRectTransform = settingsSoundEntity->AddComponent<
          RectTransformComponent>(pDevice);
        auto& settingsRenderer = settingsSoundEntity->AddComponent<UIRendererComponent>(pDevice);
        settingsRenderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/sound.png", pDevice));
        auto &button = settingsSoundEntity->AddComponent<ButtonSoundComponent>(pDevice);
        settingsRenderer.SetVisible(isSettingsMenuActive);

        settingsTransform.SetParent(settingsEntity->GetId());
        settingsRectTransform.SetAnchor(RectAnchorPreset::MiddleLeft);
        settingsRectTransform.SetAnchorOffset({100.0f, 0.0f});
        settingsRectTransform.SetSize({50.0f, 50.0f});

        button.OnClick([]{});
      }

      settingsMenuEntities.push_back(settingsSoundEntity);

      auto settingsSoundSlidebarEntity = world.CreateEntity();
      {
        auto& settingsTransform = settingsSoundSlidebarEntity->AddComponent<TransformComponent>();
        auto& settingsRectTransform = settingsSoundSlidebarEntity->AddComponent<
          RectTransformComponent>(pDevice);
        auto& settingsRenderer = settingsSoundSlidebarEntity->AddComponent<
          UIRendererComponent>(pDevice);
        settingsRenderer.SetTexture(
          textureManager.GetNewTexture(L"Assets/UI/slidebar.png", pDevice));
        settingsRenderer.SetVisible(isSettingsMenuActive);

        settingsTransform.SetParent(settingsEntity->GetId());
        settingsRectTransform.SetAnchor(RectAnchorPreset::MiddleCenter);
        settingsRectTransform.SetSize({500.0f, 20.0f});
        settingsRectTransform.SetPivotPoint({ 0.5f, -1.0f });

      }

      settingsMenuEntities.push_back(settingsSoundSlidebarEntity);

      auto volumeTextEntity = world.CreateEntity();
      {
        auto &transform = volumeTextEntity->AddComponent<TransformComponent>();
        auto &rectTransform = volumeTextEntity->AddComponent<RectTransformComponent>(pDevice);
        auto &textRenderer = volumeTextEntity->AddComponent<TextRendererComponent>(pDevice);

        rectTransform.SetAnchor(RectAnchorPreset::MiddleCenter);
        rectTransform.SetAnchorOffset({ 75.0f, -60.0f });
        textRenderer.SetFont(font);
        textRenderer.SetFontSize(50.0f);
        textRenderer.SetText(L"Volume: 50%");
        textRenderer.SetColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
        textRenderer.SetVisible(isSettingsMenuActive);
      }

      settingsMenuEntities.push_back(volumeTextEntity);

      auto settingsSoundSlidebarCursorEntity = world.CreateEntity();
      {
        auto& settingsTransform = settingsSoundSlidebarCursorEntity->AddComponent<
          TransformComponent>();
        auto& settingsRectTransform = settingsSoundSlidebarCursorEntity->AddComponent<
          RectTransformComponent>(pDevice);
        auto& settingsRenderer = settingsSoundSlidebarCursorEntity->AddComponent<
          UIRendererComponent>(pDevice);
        settingsRenderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/cursor.png", pDevice));
        settingsRenderer.SetVisible(isSettingsMenuActive);

        settingsTransform.SetParent(settingsSoundSlidebarEntity->GetId());
        settingsRectTransform.SetAnchor(RectAnchorPreset::MiddleLeft);
        settingsRectTransform.SetAnchorOffset({ settingsSoundSlidebarEntity->GetComponent<RectTransformComponent>()->GetSize().x / 2, 0.0f });
        settingsRectTransform.SetSize({50.0f, 50.0f});
        auto &slider = settingsSoundSlidebarEntity->AddComponent<SliderComponent>(pDevice, settingsSoundSlidebarCursorEntity->GetId());

        auto textRenderer = volumeTextEntity->GetComponent<TextRendererComponent>();

        slider.OnClick([&slider, textRenderer, &AudioSystem]
          {
            float newValue = slider.GetValue();
            int percentage = static_cast<int>(newValue * 100);
            textRenderer->SetText(L"Volume: " + std::to_wstring(percentage) + L"%");
            AudioSystem.SetVolume(newValue);
          });
      }

      settingsMenuEntities.push_back(settingsSoundSlidebarCursorEntity);

      // ========================================= LEADERBOARD =========================================

      auto leaderboardEntity = world.CreateEntity();
      {
        auto &settingsTransform = leaderboardEntity->AddComponent<TransformComponent>();
        auto &settingsRectTransform = leaderboardEntity->AddComponent<RectTransformComponent>(pDevice);
        auto &settingsRenderer = leaderboardEntity->AddComponent<UIRendererComponent>(pDevice);
        settingsRenderer.SetTexture(
          textureManager.GetNewTexture(L"Assets/UI/bois.jpg", pDevice));
        settingsRenderer.SetVisible(isLeaderboardActive);
        settingsRenderer.SetColor({ 1.0f, 1.0f, 1.0f, 0.75f });

        settingsRectTransform.SetAnchor(RectAnchorPreset::MiddleCenter);
        settingsRectTransform.SetSize({ 800.0f, 500.0f });
      }

      leaderboardEntities.push_back(leaderboardEntity);

      auto buttonCloseLeaderboardEntity = world.CreateEntity();
      {
        auto &settingsTransform = buttonCloseLeaderboardEntity->AddComponent<TransformComponent>();
        auto &settingsRectTransform = buttonCloseLeaderboardEntity->AddComponent<
          RectTransformComponent>(pDevice);
        auto &settingsRenderer = buttonCloseLeaderboardEntity->AddComponent<UIRendererComponent>(pDevice);
        settingsRenderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/cross.png", pDevice));
        auto &button = buttonCloseLeaderboardEntity->AddComponent<ButtonComponent>(pDevice);

        settingsTransform.SetParent(leaderboardEntity->GetId());

        settingsRenderer.SetVisible(isLeaderboardActive);

        settingsRectTransform.SetAnchor(RectAnchorPreset::TopRight);
        settingsRectTransform.SetSize({ 50.0f, 50.0f });

        ButtonComponent::ButtonColors colors;
        //colors.normal = { 0.2f, 0.0f, 1.0f, 1.0f }; // Blue
        colors.hovered = { 0.3f, 0.7f, 1.0f, 1.0f }; // Lighter blue
        colors.pressed = { 0.1f, 0.5f, 0.9f, 1.0f }; // Darker blue
        button.SetColors(colors);

        button.OnClick([this]
          {
            ShowMainMenu();
          });
      }

      leaderboardEntities.push_back(buttonCloseLeaderboardEntity);

      auto leaderboardTextEntity = world.CreateEntity();
      {
        auto &transform = leaderboardTextEntity->AddComponent<TransformComponent>();
        auto &rectTransform = leaderboardTextEntity->AddComponent<RectTransformComponent>(pDevice);
        auto &textRenderer = leaderboardTextEntity->AddComponent<TextRendererComponent>(pDevice);

        transform.SetParent(leaderboardEntity->GetId());
        rectTransform.SetAnchor(RectAnchorPreset::TopLeft);
        rectTransform.SetAnchorOffset({ padding, 0.0f });
        textRenderer.SetFont(font);
        textRenderer.SetFontSize(50.0f);
        textRenderer.SetText(L"");
        textRenderer.SetColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
        textRenderer.SetVisible(isLeaderboardActive);

        LoadLeaderboard(textRenderer);
      }

      leaderboardEntities.push_back(leaderboardTextEntity);

    }

    void SetMainMenu()
    {
      for (const std::shared_ptr<Entity>& entity : settingsMenuEntities) {
        if (entity->HasComponent<UIRendererComponent>()) {
          entity->GetComponent<UIRendererComponent>()->SetVisible(false);
        }
        if (entity->HasComponent<TextRendererComponent>()) {
          entity->GetComponent<TextRendererComponent>()->SetVisible(false);
        }
        if (entity->HasComponent<ButtonComponent>()) {
          entity->GetComponent<ButtonComponent>()->SetEnabled(false);
        }
      }

      for (const std::shared_ptr<Entity>& entity : mainMenuEntities) {
        if (entity->HasComponent<UIRendererComponent>()) {
          entity->GetComponent<UIRendererComponent>()->SetVisible(true);
        }
        if (entity->HasComponent<TextRendererComponent>()) {
          entity->GetComponent<TextRendererComponent>()->SetVisible(true);
        }
        if (entity->HasComponent<ButtonComponent>()) {
          entity->GetComponent<ButtonComponent>()->SetEnabled(true);
        }
      }
      for (const std::shared_ptr<Entity> &entity : leaderboardEntities) {
        if (entity->HasComponent<UIRendererComponent>()) {
          entity->GetComponent<UIRendererComponent>()->SetVisible(false);
        }
        if (entity->HasComponent<TextRendererComponent>()) {
          entity->GetComponent<TextRendererComponent>()->SetVisible(false);
        }
        if (entity->HasComponent<ButtonComponent>()) {
          entity->GetComponent<ButtonComponent>()->SetEnabled(false);
        }
      }
    }

    void SetSettingsMenu()
    {
      for (const std::shared_ptr<Entity>& entity : mainMenuEntities) {
        if (entity->HasComponent<UIRendererComponent>()) {
          entity->GetComponent<UIRendererComponent>()->SetVisible(false);
        }
        if (entity->HasComponent<TextRendererComponent>()) {
          entity->GetComponent<TextRendererComponent>()->SetVisible(false);
        }
        if (entity->HasComponent<ButtonComponent>()) {
          entity->GetComponent<ButtonComponent>()->SetEnabled(false);
        }
      }

      for (const std::shared_ptr<Entity>& entity : settingsMenuEntities) {
        if (entity->HasComponent<UIRendererComponent>()) {
          entity->GetComponent<UIRendererComponent>()->SetVisible(true);
        }
        if (entity->HasComponent<TextRendererComponent>()) {
          entity->GetComponent<TextRendererComponent>()->SetVisible(true);
        }
        if (entity->HasComponent<ButtonComponent>()) {
          entity->GetComponent<ButtonComponent>()->SetEnabled(true);
        }
      }
      for (const std::shared_ptr<Entity> &entity : leaderboardEntities) {
        if (entity->HasComponent<UIRendererComponent>()) {
          entity->GetComponent<UIRendererComponent>()->SetVisible(false);
        }
        if (entity->HasComponent<TextRendererComponent>()) {
          entity->GetComponent<TextRendererComponent>()->SetVisible(false);
        }
        if (entity->HasComponent<ButtonComponent>()) {
          entity->GetComponent<ButtonComponent>()->SetEnabled(false);
        }
      }
    }

    void SetLeaderboard()
    {
      for (const std::shared_ptr<Entity> &entity : settingsMenuEntities) {
        if (entity->HasComponent<UIRendererComponent>()) {
          entity->GetComponent<UIRendererComponent>()->SetVisible(false);
        }
        if (entity->HasComponent<TextRendererComponent>()) {
          entity->GetComponent<TextRendererComponent>()->SetVisible(false);
        }
        if (entity->HasComponent<ButtonComponent>()) {
          entity->GetComponent<ButtonComponent>()->SetEnabled(false);
        }
      }

      for (const std::shared_ptr<Entity> &entity : mainMenuEntities) {
        if (entity->HasComponent<UIRendererComponent>()) {
          entity->GetComponent<UIRendererComponent>()->SetVisible(false);
        }
        if (entity->HasComponent<TextRendererComponent>()) {
          entity->GetComponent<TextRendererComponent>()->SetVisible(false);
        }
        if (entity->HasComponent<ButtonComponent>()) {
          entity->GetComponent<ButtonComponent>()->SetEnabled(false);
        }
      }
      for (const std::shared_ptr<Entity> &entity : leaderboardEntities) {
        if (entity->HasComponent<UIRendererComponent>()) {
          entity->GetComponent<UIRendererComponent>()->SetVisible(true);
        }
        if (entity->HasComponent<TextRendererComponent>()) {
          entity->GetComponent<TextRendererComponent>()->SetVisible(true);
        }
        if (entity->HasComponent<ButtonComponent>()) {
          entity->GetComponent<ButtonComponent>()->SetEnabled(true);
        }
      }
    }

    void ShowMainMenu()
    {
      isMainMenuActive = true;
      isSettingsMenuActive = false;
      isLeaderboardActive = false;
      SetMainMenu();
    }

    void ShowSettingsMenu()
    {
      isMainMenuActive = false;
      isSettingsMenuActive = true;
      isLeaderboardActive = false;
      SetSettingsMenu();
    }

    void ShowLeaderboard()
    {
      isMainMenuActive = false;
      isSettingsMenuActive = false;
      isLeaderboardActive = true;
      SetLeaderboard();
    }

    void LoadLeaderboard(TextRendererComponent& textRenderer)
    {
      std::vector<std::pair<float, std::wstring>> topScores;
      std::ifstream file("scores.txt");
      if (!file.is_open()) {
        textRenderer.SetText(L"Pas de scores disponibles");
        return;
      }

      std::string line;

      while (std::getline(file, line)) {
        size_t delimiterPos = line.find('|');
        if (delimiterPos == std::string::npos) continue;
        std::string scoreString = line.substr(0, delimiterPos);
        float score = std::stof(scoreString);
        std::string dateString = line.substr(delimiterPos + 1);
        topScores.emplace_back(score, std::wstring(dateString.begin(), dateString.end()));
      }

      std::ranges::sort(topScores.begin(), topScores.end(), [](const auto& a, const auto& b)
      {
          return a.first < b.first;
      });

      if (topScores.size() > 6)
      {
        topScores.resize(6);
      }

      std::wstringstream contentStream;

      for (const auto &score : topScores)
      {
        contentStream << score.first << L" | " << score.second << L"\n";
      }

      file.close();

      textRenderer.SetText(L"TOP 6:\n" + contentStream.str());
    }
  };
}
