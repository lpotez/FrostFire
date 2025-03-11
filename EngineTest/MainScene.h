#pragma once
#include "BaseScene.h"
#include "CameraScript.h"
#include "FreeCameraScript.h"
#include "ECS/components/LightComponent.h"
#include "ECS/components/physics/ColliderComponent.h"
#include "ECS/components/physics/RigidBodyComponent.h"
#include "ECS/components/rendering/UIRendererComponent.h"
#include "ECS/components/rendering/TextRendererComponent.h"
#include "ScriptTime.h"
#include "Font/FontManager.h"
#include "ScriptSpeedometer.h"
#include "MenuScene.h"
#include <FBXEntityBuilder.h>
#include "CarDynamicScript.h"
#include "ECS/components/rendering/SpriteRenderer.h"
#include "ScriptDecompte.h"
#include "AreaSpecificsScript.h"

namespace FrostFireEngine
{
  class MainScene : public BaseScene {
  public:




    void Initialize(DispositifD3D11* pDevice) override
    {
      BaseScene::Initialize(pDevice);

      AudioSystem& player = AudioSystem::Get();
      player.PlaySound("Assets/Sounds/vroom.wav", 0.04f, true);

      World& world = GetWorld();
      TextureManager& textureManager = TextureManager::GetInstance();
      MeshManager& meshManager = MeshManager::GetInstance();
      FontManager& fontManager = FontManager::GetInstance();
      Font* font = fontManager.LoadFont(L"Assets/Fonts/font.txt", L"Assets/Fonts/font.png",
                                        pDevice);
      InputManager::GetInstance().SetUIMode(false);

      //Lumière Directionelle
      auto directionalLightEntity = world.CreateEntity();
      {

        directionalLightEntity->AddComponent<LightComponent>(
          LightType::Directional,
          XMFLOAT3(1, 1, 1), // Couleur de la lumière
          2.0f, // Intensité
          XMFLOAT3(-0.5f, -1.0f, -0.5f) // Direction vers le bas
        );
      }

      // Skybox
      {
        auto&           renderingSystem = *world.GetSystem<RenderingSystem>();
        TextureManager& textureManager = TextureManager::GetInstance();
        std::wstring    texturePath = L"../EngineTest/Assets/sky.dds";
        auto            texture = textureManager.GetNewTexture(texturePath, pDevice);
        renderingSystem.SetSkyboxTexture(texture);
      }

      //Cadran de vitesse
      auto speedometer = world.CreateEntity();
      {
        auto& transform = speedometer->AddComponent<TransformComponent>();
        auto& rectTransform = speedometer->AddComponent<RectTransformComponent>(pDevice);
        auto& spriteRenderer = speedometer->AddComponent<UIRendererComponent>(
          pDevice
        );
        auto texture = textureManager.GetNewTexture(L"Assets/UI/cadran.png", pDevice);
        spriteRenderer.SetTexture(texture);

        spriteRenderer.SetOpaque(false);
        spriteRenderer.SetColor({1.0, 1.0, 1.0, 0.5});
        rectTransform.SetAnchor(RectAnchorPreset::BottomRight);
        rectTransform.SetPivotPoint(XMFLOAT2(0.0f, 0.0f));

        rectTransform.SetSize({static_cast<float>(texture->GetWidth()),
          static_cast<float>(texture->GetHeight())});

      }

      //Aiguille du cadran
      auto speedometerIndicator = world.CreateEntity();
      {
        auto& transform = speedometerIndicator->AddComponent<TransformComponent>();
        auto& rectTransform = speedometerIndicator->AddComponent<RectTransformComponent>(pDevice);
        auto& spriteRenderer = speedometerIndicator->AddComponent<UIRendererComponent>(
          pDevice
        );
        auto texture = textureManager.GetNewTexture(L"Assets/UI/aiguille.png", pDevice);
        spriteRenderer.SetTexture(texture);
        spriteRenderer.SetOpaque(false);

        spriteRenderer.SetColor({1.0, 1.0, 1.0, 0.5});


        transform.SetParent(speedometer->GetId());

        rectTransform.SetAnchor(RectAnchorPreset::MiddleCenter);
        rectTransform.SetAnchorOffset(XMFLOAT2(0.f, 40.0f));
        rectTransform.SetScale(XMFLOAT2(0.5f, 0.5f));
        rectTransform.SetPivotPoint(XMFLOAT2(0.25f, 0.07f));

        rectTransform.SetSize({static_cast<float>(texture->GetWidth()),
          static_cast<float>(texture->GetHeight())});

      }

      // Décompte
      auto decompteEntity = world.CreateEntity();
      {
        auto& transform = decompteEntity->AddComponent<TransformComponent>();
        auto& rectTransform = decompteEntity->AddComponent<RectTransformComponent>(pDevice);
        auto& textRenderer = decompteEntity->AddComponent<TextRendererComponent>(pDevice);
        textRenderer.SetFont(font);
        textRenderer.SetText(L"");
        textRenderer.SetColor(XMFLOAT4(1.0f, 0.0f, 0.0f, 0.5f));
        textRenderer.SetFontSize(100.0f);
        rectTransform.SetAnchor(RectAnchorPreset::MiddleCenter);
        rectTransform.SetAnchorOffset({0.0f, -30.0f});
        rectTransform.SetSize({textRenderer.GetContentWidth(), textRenderer.GetContentHeight()});

    

      }

      
      // Temps écoulé
      {
        auto entity = world.CreateEntity();

        auto& transform = entity->AddComponent<TransformComponent>();
        auto& rectTransform = entity->AddComponent<RectTransformComponent>(pDevice);
        auto& textRenderer = entity->AddComponent<TextRendererComponent>(pDevice);
        
        textRenderer.SetFont(font);
        textRenderer.SetText(L"FrostFire");
        textRenderer.SetOpaque(false);
        textRenderer.SetColor(XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f));
        textRenderer.SetFontSize(50.0f);
        
        rectTransform.SetAnchor(RectAnchorPreset::TopRight);
        rectTransform.SetAnchorOffset({-10.0f, 10.0f});

        rectTransform.SetSize({textRenderer.GetContentWidth(), textRenderer.GetContentHeight()});
        entity->AddComponent<ScriptTime>();
      }
      

      // Screen Pause
      auto screenPause = world.CreateEntity();
      {
        auto& transform = screenPause->AddComponent<TransformComponent>();
        auto& rectTransform = screenPause->AddComponent<RectTransformComponent>(pDevice);
        auto& spriteRenderer = screenPause->AddComponent<UIRendererComponent>(
          pDevice
        );
        auto texture = textureManager.GetNewTexture(L"Assets/UI/ScreenPause.dds", pDevice);
        spriteRenderer.SetTexture(texture);
        rectTransform.SetSizingMode(RectSizingMode::Stretch);
        spriteRenderer.SetVisible(false);

        rectTransform.SetSize(
          {static_cast<float>(std::min(pDevice->GetWidth(), texture->GetWidth())),
            static_cast<float>(std::min(pDevice->GetHeight(), texture->GetHeight()))});

      }
      // Screen Pause - Quit Game Button
      auto buttonQuitEntity = world.CreateEntity();
      {
        auto& settingsTransform = buttonQuitEntity->AddComponent<TransformComponent>();
        auto& settingsRectTransform = buttonQuitEntity->AddComponent<
          RectTransformComponent>(pDevice);
        auto& settingsRenderer = buttonQuitEntity->AddComponent<UIRendererComponent>(pDevice);

        settingsRenderer.SetTexture(
          textureManager.GetNewTexture(L"Assets/UI/boutonCraquele.jpg", pDevice));

        auto& button = buttonQuitEntity->AddComponent<ButtonComponent>(pDevice);

        settingsTransform.SetParent(screenPause->GetId());
        settingsRenderer.SetVisible(false);

        settingsRectTransform.SetAnchor(RectAnchorPreset::BottomCenter);
        settingsRectTransform.SetAnchorOffset({150.0f, -20.0f});
        settingsRectTransform.SetSize({250.0f, 50.0f});

        ButtonComponent::ButtonColors colors;
        colors.normal = { 0.2f, 0.6f, 1.0f, 1.0f }; // Blue
        colors.hovered = {0.3f, 0.7f, 1.0f, 1.0f}; // Lighter blue
        colors.pressed = {0.1f, 0.5f, 0.9f, 1.0f}; // Darker blue
        button.SetColors(colors);

        button.OnClick([this, &world]
        {
          InputManager::GetInstance().UnlockCursor();
          DestroyWindow(GetActiveWindow());
        });
      }

      //Screen Pause - Quit Game Button
      auto entityQuitText = world.CreateEntity();
      {
        auto& transform = entityQuitText->AddComponent<TransformComponent>();
        auto& rectTransform = entityQuitText->AddComponent<RectTransformComponent>(pDevice);
        auto& textRenderer = entityQuitText->AddComponent<TextRendererComponent>(pDevice);

        transform.SetParent(buttonQuitEntity->GetId());

        textRenderer.SetVisible(false);
        textRenderer.SetFont(font);
        textRenderer.SetText(L"QUITTER");
        auto size = textRenderer.SetFontSize(50.0f);
        rectTransform.SetSize(size);
        textRenderer.SetColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

        rectTransform.SetAnchor(RectAnchorPreset::MiddleCenter);
      }

      // Screen Pause - Back to Menu Button
      auto buttonBackToMenuEntity = world.CreateEntity();
      {
        auto& settingsTransform = buttonBackToMenuEntity->AddComponent<TransformComponent>();
        auto& settingsRectTransform = buttonBackToMenuEntity->AddComponent<
          RectTransformComponent>(pDevice);
        auto& settingsRenderer = buttonBackToMenuEntity->AddComponent<UIRendererComponent>(pDevice);

        settingsRenderer.SetTexture(
          textureManager.GetNewTexture(L"Assets/UI/boutonCraquele.jpg", pDevice));

        auto& button = buttonBackToMenuEntity->AddComponent<ButtonComponent>(pDevice);

        settingsTransform.SetParent(screenPause->GetId());
        settingsRenderer.SetVisible(false);

        settingsRectTransform.SetAnchor(RectAnchorPreset::BottomCenter);
        settingsRectTransform.SetAnchorOffset({-150.0f, -20.0f});
        settingsRectTransform.SetSize({250.0f, 50.0f});

        ButtonComponent::ButtonColors colors;
        colors.normal = { 0.2f, 0.6f, 1.0f, 1.0f }; // Blue
        colors.hovered = {0.3f, 0.7f, 1.0f, 1.0f}; // Lighter blue
        colors.pressed = {0.1f, 0.5f, 0.9f, 1.0f}; // Darker blue
        button.SetColors(colors);

        button.OnClick([]
        {
          InputManager::GetInstance().SetUIMode(true);

          auto& sceneManager = SceneManager::GetInstance();
          sceneManager.SetActiveScene<MenuScene>(EngineWindows::GetInstance().GetDevice());

        });
      }

      auto entityBackToMenuText = world.CreateEntity();
      {
        auto& transform = entityBackToMenuText->AddComponent<TransformComponent>();
        auto& rectTransform = entityBackToMenuText->AddComponent<RectTransformComponent>(pDevice);
        auto& textRenderer = entityBackToMenuText->AddComponent<TextRendererComponent>(pDevice);

        transform.SetParent(buttonBackToMenuEntity->GetId());

        textRenderer.SetVisible(false);
        textRenderer.SetFont(font);
        textRenderer.SetText(L"ACCUEIL");
        auto size = textRenderer.SetFontSize(50.0f);
        rectTransform.SetSize(size);
        textRenderer.SetColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

        rectTransform.SetAnchor(RectAnchorPreset::MiddleCenter);
      }

      // Victory Screen
      auto screenGameOver = world.CreateEntity();
      {
        auto& transform = screenGameOver->AddComponent<TransformComponent>();
        auto& rectTransform = screenGameOver->AddComponent<RectTransformComponent>(pDevice);
        auto& spriteRenderer = screenGameOver->AddComponent<UIRendererComponent>(
          pDevice
        );
        auto texture = textureManager.GetNewTexture(L"Assets/UI/VictoryScreen.dds", pDevice);
        spriteRenderer.SetTexture(texture);
        rectTransform.SetSizingMode(RectSizingMode::Stretch);
        spriteRenderer.SetVisible(false);

        rectTransform.SetSize(
          {static_cast<float>(std::min(pDevice->GetWidth(), texture->GetWidth())),
            static_cast<float>(std::min(pDevice->GetHeight(), texture->GetHeight()))});

      }

      // Load Map
      {

        FBXEntityBuilder                builder(pDevice);
        FBXEntityBuilder::BuildSettings settings;
        settings.fbxPath = "Assets/objects/test/MapFinale2.fbx";
        settings.scaleFactor = 0.01f;

        settings.flipUVs = true;
        settings.generateColliders = true;
        settings.colliderMeshType = ColliderComponent::MeshType::Triangle;
        if (auto buildResult = builder.BuildFromFile(settings); buildResult.success) {
          if (buildResult.rootEntity->GetId() != INVALID_ENTITY_ID) {
            TransformComponent& transform = *buildResult.rootEntity->GetComponent<
              TransformComponent>();

            transform.SetPosition({0.0f, -30.0f, 0.0f});

            buildResult.rootEntity->AddComponent<ColliderComponent>(
              ColliderComponent(ColliderComponent::Type::Box));
            buildResult.rootEntity->GetComponent<ColliderComponent>()->Initialize({
              1.0f, -1.0f, 1.0f
            });
            buildResult.rootEntity->AddComponent<RigidBodyComponent>(
              RigidBodyComponent::Type::Static).Initialize();
          }
        }


        auto entity = world.GetEntity(88);
        auto mapRenderer = entity->GetComponent<PBRRenderer>();
        //mapRenderer->SetAlbedoTexture(textureManager.GetNewTexture(L"Assets/objects/test/terrain/color.jpg", pDevice));
        mapRenderer->SetAOMap(textureManager.GetNewTexture(L"Assets/objects/test/terrain/ao.jpg", pDevice));
        mapRenderer->SetNormalMap(textureManager.GetNewTexture(L"Assets/objects/test/terrain/normal.jpg", pDevice));
        mapRenderer->SetMetallicRoughnessMap(textureManager.GetNewTexture(L"Assets/objects/test/terrain/roughness.jpg", pDevice));
      }

      //Zones hors-piste
      //Lacs de glace et de lave
      {

        FBXEntityBuilder                builder(pDevice);
        FBXEntityBuilder::BuildSettings settings;
        settings.fbxPath = "Assets/objects/test/DeadZones.fbx";
        settings.scaleFactor = 0.01f;

        settings.flipUVs = true;
        settings.generateColliders = true;
        settings.colliderMeshType = ColliderComponent::MeshType::Triangle;
        if (auto buildResult = builder.BuildFromFile(settings); buildResult.success) {
          if (buildResult.rootEntity->GetId() != INVALID_ENTITY_ID) {
            TransformComponent &transform = *buildResult.rootEntity->GetComponent<
              TransformComponent>();

            transform.SetPosition({ 0.0f, -30.0f, 0.0f });

            buildResult.rootEntity->AddComponent<ColliderComponent>(
              ColliderComponent(ColliderComponent::Type::Box));
            buildResult.rootEntity->GetComponent<ColliderComponent>()->Initialize({
              1.0f, -1.0f, 1.0f
              });
            buildResult.rootEntity->AddComponent<RigidBodyComponent>(
              RigidBodyComponent::Type::Static).Initialize();

            buildResult.rootEntity->AddComponent<DeadZoneScript>();
          }
        }

      }
      //Zone interdite à côté du premier checkpoint
      {
        auto     cubeEntity = world.CreateEntity();
        EntityId cubeId = cubeEntity->GetId();

        auto cubeTransform = TransformComponent({ 79.5f, -20.5f, -2.5f });
        cubeTransform.SetScale({ 20.0f, 0.01f, 39.f });

        cubeEntity->AddComponent<TransformComponent>(cubeTransform);

        auto &meshComponent = cubeEntity->AddComponent<MeshComponent>();
        meshComponent.SetMesh(meshManager.GetCubeMesh(pDevice->GetD3DDevice(), 0.5f));

        cubeEntity->AddComponent<
          ColliderComponent>(ColliderComponent(ColliderComponent::Type::Box));
        cubeEntity->GetComponent<ColliderComponent>()->Initialize({ 1.0f, 0.5f, 1.0f });
        auto rigidBody = RigidBodyComponent(RigidBodyComponent::Type::Static, true);
        cubeEntity->AddComponent<RigidBodyComponent>(rigidBody);
        cubeEntity->GetComponent<RigidBodyComponent>()->Initialize();
        cubeEntity->AddComponent<DeadZoneScript>();

      }

      //Zone interdite de la maison
      {
        auto     cubeEntity = world.CreateEntity();
        EntityId cubeId = cubeEntity->GetId();

        auto cubeTransform = TransformComponent({ 62.0f, -20.5f, -10.f });
        cubeTransform.SetScale({ 30.0f, 0.01f, 70.f });

        cubeTransform.SetRotation({ 0, 0.0436194, 0, 0.9990482 });

        cubeEntity->AddComponent<TransformComponent>(cubeTransform);

        auto &meshComponent = cubeEntity->AddComponent<MeshComponent>();
        meshComponent.SetMesh(meshManager.GetCubeMesh(pDevice->GetD3DDevice(), 0.5f));

        cubeEntity->AddComponent<
          ColliderComponent>(ColliderComponent(ColliderComponent::Type::Box));
        cubeEntity->GetComponent<ColliderComponent>()->Initialize({ 1.0f, 0.5f, 1.0f });
        auto rigidBody = RigidBodyComponent(RigidBodyComponent::Type::Static, true);
        cubeEntity->AddComponent<RigidBodyComponent>(rigidBody);
        cubeEntity->GetComponent<RigidBodyComponent>()->Initialize();
        cubeEntity->AddComponent<DeadZoneScript>();

      }

      //Zone interdite - pour les p'tits malins qui essayeraient de couper la route
      {
        auto     cubeEntity = world.CreateEntity();
        EntityId cubeId = cubeEntity->GetId();

        auto cubeTransform = TransformComponent({ 50.0f, -10.0f, 30.f });
        cubeTransform.SetScale({ 50.0f, 0.01f, 100.f });

        cubeTransform.SetRotation({ 0, 0.0436194, 0, 0.9990482 });

        cubeEntity->AddComponent<TransformComponent>(cubeTransform);

        auto &meshComponent = cubeEntity->AddComponent<MeshComponent>();
        meshComponent.SetMesh(meshManager.GetCubeMesh(pDevice->GetD3DDevice(), 0.5f));

        cubeEntity->AddComponent<
          ColliderComponent>(ColliderComponent(ColliderComponent::Type::Box));
        cubeEntity->GetComponent<ColliderComponent>()->Initialize({ 1.0f, 0.5f, 1.0f });
        auto rigidBody = RigidBodyComponent(RigidBodyComponent::Type::Static, true);
        cubeEntity->AddComponent<RigidBodyComponent>(rigidBody);
        cubeEntity->GetComponent<RigidBodyComponent>()->Initialize();
        cubeEntity->AddComponent<DeadZoneScript>();

      }

      //Checkpoints
      int *checkPointManager = new int(0);
      std::vector<EntityId> checkpointList;

      //Premier checkpoint - et l'arrivée accesoirement
      {
        auto     cubeEntity = world.CreateEntity();
        EntityId cubeId = cubeEntity->GetId();

        auto cubeTransform = TransformComponent({87.5f, -17.0f, -33.0f});
        cubeTransform.SetScale({15.0f, 14.0f, 0.01f});

        cubeTransform.SetRotation({0, 0.7071068, 0, 0.7071068});

        cubeEntity->AddComponent<TransformComponent>(cubeTransform);

        auto& meshComponent = cubeEntity->AddComponent<MeshComponent>();
        meshComponent.SetMesh(meshManager.GetCubeMesh(pDevice->GetD3DDevice(), 0.5f));
        auto& renderer = cubeEntity->AddComponent<PBRRenderer>(pDevice->GetD3DDevice());
        renderer.SetBaseColor({0.0f, 0.8f, 1.0f, 0.5f});

        renderer.SetOpaque(false);

        cubeEntity->AddComponent<
          ColliderComponent>(ColliderComponent(ColliderComponent::Type::Box));
        cubeEntity->GetComponent<ColliderComponent>()->Initialize({1.0f, 0.5f, 1.0f});
        auto rigidBody = RigidBodyComponent(RigidBodyComponent::Type::Static, true);
        cubeEntity->AddComponent<RigidBodyComponent>(rigidBody);
        cubeEntity->GetComponent<RigidBodyComponent>()->Initialize();
        auto script = CheckpointScript(0, checkPointManager);
        script.SetPosition({87.5f, -20.0, -35.5f});
        cubeEntity->AddComponent<CheckpointScript>(script);
        checkpointList.push_back(cubeId);
        cubeEntity->AddComponent<AreaSpecificsScript>(directionalLightEntity->GetId(), AreaSpecificsScript::Specifics::LIGHTER, true);
      }

      //Deuxieme checkpoint - dans le tunnel - retire la lumière
      {
        auto     cubeEntity = world.CreateEntity();
        EntityId cubeId = cubeEntity->GetId();

        auto cubeTransform = TransformComponent({-16.3f, -15.5f, 35.f });
        cubeTransform.SetScale({22.0f, 12.0f, 0.01f});
        cubeTransform.SetRotation({ 0, 0.0610485, 0, 0.9981348 });
        //cubeTransform.SetRotation({ 0, 0.9238795, 0, 0.3826834 });
        cubeEntity->AddComponent<TransformComponent>(cubeTransform);


        auto& meshComponent = cubeEntity->AddComponent<MeshComponent>();
        meshComponent.SetMesh(meshManager.GetCubeMesh(pDevice->GetD3DDevice(), 0.5f));
        auto& renderer = cubeEntity->AddComponent<PBRRenderer>(pDevice->GetD3DDevice());
        renderer.SetBaseColor({1.0f, 0.5f, 0.0f, 0.5f});

        renderer.SetOpaque(false);


        cubeEntity->AddComponent<
          ColliderComponent>(ColliderComponent(ColliderComponent::Type::Box));
        cubeEntity->GetComponent<ColliderComponent>()->Initialize({2.0f, 2.0f, 2.0f});
        auto rigidBody = RigidBodyComponent(RigidBodyComponent::Type::Static, true);
        cubeEntity->AddComponent<RigidBodyComponent>(rigidBody);
        cubeEntity->GetComponent<RigidBodyComponent>()->Initialize();
        auto script = CheckpointScript(1, checkPointManager);
        script.SetPosition({ -16.3f, -20, 35.0f});
        cubeEntity->AddComponent<CheckpointScript>(script);
        cubeEntity->AddComponent<AreaSpecificsScript>(directionalLightEntity->GetId(),
                                                      AreaSpecificsScript::Specifics::DARKER);
        checkpointList.push_back(cubeId);
      }

      //Troisième checkpoint - squelette
      {
        auto     cubeEntity = world.CreateEntity();
        EntityId cubeId = cubeEntity->GetId();

        auto cubeTransform = TransformComponent({-56.f, -21.0f, 23.0f});
        cubeTransform.SetScale({17.0f, 11.2f, 0.01f});

        cubeTransform.SetRotation({0, 1, 0, 0});
        cubeEntity->AddComponent<TransformComponent>(cubeTransform);


        auto& meshComponent = cubeEntity->AddComponent<MeshComponent>();
        meshComponent.SetMesh(meshManager.GetSphereMesh(pDevice->GetD3DDevice(), 0.5f));
        auto& renderer = cubeEntity->AddComponent<PBRRenderer>(pDevice->GetD3DDevice());
        renderer.SetBaseColor({1.0f, 0.5f, 0.0f, 0.5f});

        renderer.SetOpaque(false);


        cubeEntity->AddComponent<
          ColliderComponent>(ColliderComponent(ColliderComponent::Type::Box));
        cubeEntity->GetComponent<ColliderComponent>()->Initialize({1.0f, 0.5f, 1.0f});
        auto rigidBody = RigidBodyComponent(RigidBodyComponent::Type::Static, true);
        cubeEntity->AddComponent<RigidBodyComponent>(rigidBody);
        cubeEntity->GetComponent<RigidBodyComponent>()->Initialize();
        auto script = CheckpointScript(2, checkPointManager);
        script.SetPosition({-56.f, -20.0f, 23.0f});
        cubeEntity->AddComponent<CheckpointScript>(script);
        checkpointList.push_back(cubeId);
      }
      //Dernier checkpoint - dans le tunnel en haut - rapporte la lumière
      {
        auto     cubeEntity = world.CreateEntity();
        EntityId cubeId = cubeEntity->GetId();
        auto     cubeTransform = TransformComponent({-15.0f, 3.0f, 16.5f});
        cubeTransform.SetScale({16.0f, 15.0f, 0.01f});
        cubeTransform.SetRotation({0.f, 0.7071068f, 0, 0.7071068f});

        cubeEntity->AddComponent<TransformComponent>(cubeTransform);

        auto& meshComponent = cubeEntity->AddComponent<MeshComponent>();
        meshComponent.SetMesh(meshManager.GetCubeMesh(pDevice->GetD3DDevice(), 0.5f));
        auto& renderer = cubeEntity->AddComponent<PBRRenderer>(pDevice->GetD3DDevice());
        renderer.SetBaseColor({0.0f, 0.8f, 0.5f, 0.5f});
        renderer.SetOpaque(false);

        cubeEntity->AddComponent<
          ColliderComponent>(ColliderComponent(ColliderComponent::Type::Box));
        cubeEntity->GetComponent<ColliderComponent>()->Initialize({1.0f, 0.5f, 1.0f});
        auto rigidBody = RigidBodyComponent(RigidBodyComponent::Type::Static, true);
        cubeEntity->AddComponent<RigidBodyComponent>(rigidBody);
        cubeEntity->GetComponent<RigidBodyComponent>()->Initialize();
        auto script = CheckpointScript(3, checkPointManager);
        script.SetPosition({-15.f, -1.f, 16.5f});
        cubeEntity->AddComponent<CheckpointScript>(script);
        cubeEntity->AddComponent<AreaSpecificsScript>(directionalLightEntity->GetId(),
                                                      AreaSpecificsScript::Specifics::LIGHTER);
        checkpointList.push_back(cubeId);
      }


      //AreaSpecific
      //Ramene la lumière si on fait demi-tour dans le tunnel
      {
        auto     cubeEntity = world.CreateEntity();
        EntityId cubeId = cubeEntity->GetId();
        auto     cubeTransform = TransformComponent({ -20.0f, 3.0f, 16.5f });
        cubeTransform.SetScale({ 16.0f, 15.0f, 0.01f });
        cubeTransform.SetRotation({ 0.f, 0.7071068f, 0, 0.7071068f });

        cubeEntity->AddComponent<TransformComponent>(cubeTransform);

        auto &meshComponent = cubeEntity->AddComponent<MeshComponent>();
        meshComponent.SetMesh(meshManager.GetCubeMesh(pDevice->GetD3DDevice(), 0.5f));
    

        cubeEntity->AddComponent<
          ColliderComponent>(ColliderComponent(ColliderComponent::Type::Box));
        cubeEntity->GetComponent<ColliderComponent>()->Initialize({ 1.0f, 0.5f, 1.0f });
        auto rigidBody = RigidBodyComponent(RigidBodyComponent::Type::Static, true);
        cubeEntity->AddComponent<RigidBodyComponent>(rigidBody);
        cubeEntity->GetComponent<RigidBodyComponent>()->Initialize();
        cubeEntity->AddComponent<AreaSpecificsScript>(directionalLightEntity->GetId(), AreaSpecificsScript::Specifics::DARKER);

      }

      //Assombrit la scène quand on revient dans la zone de lave
      {
        auto     cubeEntity = world.CreateEntity();
        EntityId cubeId = cubeEntity->GetId();

        auto     cubeTransform = TransformComponent({ -22.5f, -20.5f, -24.f });
        cubeTransform.SetScale({ 20.0f, 12.0f, 0.01f });
        cubeTransform.SetRotation({ 0, -0.3826834, 0, 0.9238795 });
        cubeEntity->AddComponent<TransformComponent>(cubeTransform);

        auto &meshComponent = cubeEntity->AddComponent<MeshComponent>();
        meshComponent.SetMesh(meshManager.GetCubeMesh(pDevice->GetD3DDevice(), 0.5f));
      

        cubeEntity->AddComponent<
          ColliderComponent>(ColliderComponent(ColliderComponent::Type::Box));
        cubeEntity->GetComponent<ColliderComponent>()->Initialize({ 1.0f, 0.5f, 1.0f });
        auto rigidBody = RigidBodyComponent(RigidBodyComponent::Type::Static, true);
        cubeEntity->AddComponent<RigidBodyComponent>(rigidBody);
        cubeEntity->GetComponent<RigidBodyComponent>()->Initialize();
     
        cubeEntity->AddComponent<AreaSpecificsScript>(directionalLightEntity->GetId(), AreaSpecificsScript::Specifics::LIGHTER);
      
      }

      // Load Vehicle
      {
        FBXEntityBuilder                builder(pDevice);
        FBXEntityBuilder::BuildSettings settings;
        settings.fbxPath = "Assets/objects/test/LP_VehicleA.fbx";
        settings.scaleFactor = 0.008f;
        settings.generateColliders = true;
        if (auto buildResult = builder.BuildFromFile(settings); buildResult.success) {
          if (buildResult.rootEntity->GetId() != INVALID_ENTITY_ID) {
            TransformComponent& transform = *buildResult.rootEntity->GetComponent<
              TransformComponent>();
            transform.SetRotation({0.f, -0.7071068f, 0, 0.7071068f});
            auto parent = world.CreateEntity();

            TransformComponent& transformParent = parent->AddComponent<TransformComponent>();
            transformParent.SetPosition({87.5f, -20.5f, -35.5f});

            transformParent.AddChild(buildResult.rootEntity->GetId());
            transformParent.SetRotation({0.f, 0.7071068f, 0.f, 0.7071068f});

            parent->AddComponent<RigidBodyComponent>(RigidBodyComponent::Type::Dynamic).
                    Initialize();
            parent->AddComponent<CarDynamicScript>();
            parent->AddComponent<ScriptPauseManager>(screenPause->GetId());
            parent->AddComponent<VictoryScript>(checkPointManager, checkpointList,
                                                screenGameOver->GetId());
            auto scriptCamera = freeCameraEntity->AddComponent<CameraScript>(
              cameraTPSEntity->GetId(), cameraFPSEntity->GetId(), freeCameraEntity->GetId(),
              buildResult.rootEntity->GetId()
            );

            decompteEntity->AddComponent<ScriptDecompte>(parent->GetId());

            speedometerIndicator->AddComponent<ScriptSpeedometer>(parent->GetId());

            decompteEntity->AddComponent<ScriptDecompte>(parent->GetId());

          }
        }
      }

      //Zone de boost

      {
        auto     cubeEntity = world.CreateEntity();
        EntityId cubeId = cubeEntity->GetId();

        auto cubeTransform = TransformComponent({-106.5f, -17.0f, 17.0f});
        cubeTransform.SetScale({13.f, 12.0f, 0.01f});

        cubeTransform.SetRotation({0, 0.7071068, 0, 0.7071068});

        cubeEntity->AddComponent<TransformComponent>(cubeTransform);

        auto& meshComponent = cubeEntity->AddComponent<MeshComponent>();
        meshComponent.SetMesh(meshManager.GetCubeMesh(pDevice->GetD3DDevice(), 0.5f));
        /*
        auto &renderer = cubeEntity->AddComponent<PBRRenderer>(pDevice->GetD3DDevice());
        renderer.SetBaseColor({ 1.0f, 0.0f, 0.1f, 0.5f });

        renderer.SetOpaque(false);
        */
        cubeEntity->AddComponent<
          ColliderComponent>(ColliderComponent(ColliderComponent::Type::Box));
        cubeEntity->GetComponent<ColliderComponent>()->Initialize({1.0f, 0.5f, 1.0f});
        auto rigidBody = RigidBodyComponent(RigidBodyComponent::Type::Static, true);
        cubeEntity->AddComponent<RigidBodyComponent>(rigidBody);
        cubeEntity->GetComponent<RigidBodyComponent>()->Initialize();
        cubeEntity->AddComponent<BoostScript>(1.0, 100);

      }

      //Billboards
      //Symbole du boost - il aurait pu avoir le script BoostScript  
      {
        auto  logoEntity = world.CreateEntity();
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        transform.SetScale({ 3, 3, 3 });
        transform.SetPosition({ -105.9f, -19.0f, 17.0f });
        auto &renderer = logoEntity->AddComponent<SpriteRenderer>(pDevice->GetD3DDevice());
        renderer.SetBillboard(true);
        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/Boost.png", pDevice));

      }

      
     
      //Tableau 1  - Près d'une bougie violette
      {
        auto  logoEntity = world.CreateEntity();
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        transform.SetScale({ 10, 10, 10 });
        transform.SetPosition({ -49 , -16 , 45.902 });
        auto &renderer = logoEntity->AddComponent<SpriteRenderer>(pDevice->GetD3DDevice());
        renderer.SetBillboard(true);
        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/Tableau.png", pDevice));

      }

      //Tableau 1  - Près d'une bougie orange
      {
        auto  logoEntity = world.CreateEntity();
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        transform.SetScale({ 10, 10, 10 });
        transform.SetPosition({ -42 , -16 , -29 });
        auto &renderer = logoEntity->AddComponent<SpriteRenderer>(pDevice->GetD3DDevice());
        renderer.SetBillboard(true);
        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/Tableau.png", pDevice));

      }

      //Bougie Violette 1 - En sortant du tunnel
      {
        auto  logoEntity = world.CreateEntity();
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        transform.SetScale({ 1, 1, 1 });
        transform.SetPosition({ -51.9f, -20.4f, 81.0f });
        auto &renderer = logoEntity->AddComponent<SpriteRenderer>(pDevice->GetD3DDevice());
        renderer.SetBillboard(true);

        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/Bougie.png", pDevice));

      }

      //Bougie Violette 2 - Un peu plus loin
      {
        auto  logoEntity = world.CreateEntity();
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        transform.SetScale({ 1, 1, 1 });
        transform.SetPosition({ -79.7f, -20.4f, 69.0f });
        auto &renderer = logoEntity->AddComponent<SpriteRenderer>(pDevice->GetD3DDevice());
        renderer.SetBillboard(true);

        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/Bougie.png", pDevice));

      }

      //Bougie Violette 3 - Au fond à droite
      {
        auto  logoEntity = world.CreateEntity();
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        transform.SetScale({ 1, 1, 1 });
        transform.SetPosition({ -118.3f, -20.4f, 77.6f });
        auto &renderer = logoEntity->AddComponent<SpriteRenderer>(pDevice->GetD3DDevice());
        renderer.SetBillboard(true);

        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/Bougie.png", pDevice));

      }

      //Bougie Violette 4 - Au niveau des tombes
      {
        auto  logoEntity = world.CreateEntity();
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        transform.SetScale({ 1, 1, 1 });
        transform.SetPosition({ -105.0f, -20.4f, 59.0f });
        auto &renderer = logoEntity->AddComponent<SpriteRenderer>(pDevice->GetD3DDevice());
        renderer.SetBillboard(true);

        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/Bougie.png", pDevice));

      }

      //Bougie Violette 5 - Dans le squelette
      {
        auto  logoEntity = world.CreateEntity();
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        transform.SetScale({ 1, 1, 1 });
        transform.SetPosition({ -120.3f, -20.4f, 30.5f });
        auto &renderer = logoEntity->AddComponent<SpriteRenderer>(pDevice->GetD3DDevice());
        renderer.SetBillboard(true);

        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/Bougie.png", pDevice));

      }

      //Bougie Violette 6 - Quelque part
      {
        auto  logoEntity = world.CreateEntity();
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        transform.SetScale({ 1, 1, 1 });
        transform.SetPosition({ -54.3f, -20.4f, 45.4f });
        auto &renderer = logoEntity->AddComponent<SpriteRenderer>(pDevice->GetD3DDevice());
        renderer.SetBillboard(true);

        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/Bougie.png", pDevice));

      }

      //Bougie Violette 7 - Près du tableau
      {
        auto  logoEntity = world.CreateEntity();
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        transform.SetScale({ 1, 1, 1 });
        transform.SetPosition({ -43.8f, -20.4f, -25.4f });
        auto &renderer = logoEntity->AddComponent<SpriteRenderer>(pDevice->GetD3DDevice());
        renderer.SetBillboard(true);

        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/Bougie.png", pDevice));

      }

      //Bougie Orange 1 - Sur le squelette
      {
        auto  logoEntity = world.CreateEntity();
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        transform.SetScale({ 1, 1, 1 });
        transform.SetPosition({ -56.f, -15.9f, 27.5f });
        auto &renderer = logoEntity->AddComponent<SpriteRenderer>(pDevice->GetD3DDevice());
        renderer.SetBillboard(true);

        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/BougieOrange.png", pDevice));

      }

      //Bougie Orange 2 - Près du tableau
      {
        auto  logoEntity = world.CreateEntity();
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        transform.SetScale({ 1, 1, 1 });
        transform.SetPosition({ -43.8f, -20.4f, -25.4f });
        auto &renderer = logoEntity->AddComponent<SpriteRenderer>(pDevice->GetD3DDevice());
        renderer.SetBillboard(true);

        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/BougieOrange.png", pDevice));

      }

      //Bougie Orange 3 - Toute seule sur le chemin
      {
        auto  logoEntity = world.CreateEntity();
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        transform.SetScale({ 1, 1, 1 });
        transform.SetPosition({ -95.2f, -20.4f, -39.2f });
        auto &renderer = logoEntity->AddComponent<SpriteRenderer>(pDevice->GetD3DDevice());
        renderer.SetBillboard(true);

        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/BougieOrange.png", pDevice));

      }

      //Bougie Orange 4 - Près du petit panneau avant le pont
      {
        auto  logoEntity = world.CreateEntity();
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        transform.SetScale({ 1, 1, 1 });
        transform.SetPosition({ -127.0f, -20.4f, -28.3f });
        auto &renderer = logoEntity->AddComponent<SpriteRenderer>(pDevice->GetD3DDevice());
        renderer.SetBillboard(true);

        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/BougieOrange.png", pDevice));

      }
  
      //Bougie Orange 5 - Près du grand panneau après le pont
      {
        auto  logoEntity = world.CreateEntity();
        auto &transform = logoEntity->AddComponent<TransformComponent>();
        transform.SetScale({ 1, 1, 1 });
        transform.SetPosition({ -120.3f, -20.4f, 13.9f });
        auto &renderer = logoEntity->AddComponent<SpriteRenderer>(pDevice->GetD3DDevice());
        renderer.SetBillboard(true);

        renderer.SetTexture(textureManager.GetNewTexture(L"Assets/UI/BougieOrange.png", pDevice));

      }


      world.BuildOctree();
    }

  protected:
    void SetupCamera() override
    {
      World& world = GetWorld();

      constexpr float fieldOfView = XM_PI / 4;
      constexpr float aspectRatio = 16.0f / 9.0f;
      constexpr float nearPlane = 0.1f;
      constexpr float farPlane = 1000.0f;

      // Create Cameras
      freeCameraEntity = world.CreateEntity();
      freeCameraEntity->AddComponent<TransformComponent>(XMFLOAT3{0.0f, 10.0f, -30.0f});
      freeCameraEntity->AddComponent<CameraComponent>(
        CameraComponent(fieldOfView, aspectRatio, nearPlane, farPlane, freeCameraEntity->GetId()));
      auto& freeCameraScript = freeCameraEntity->AddComponent<FreeCameraScript>();
      freeCameraScript.SetEnabled(false);
      cameraTPSEntity = world.CreateEntity();
      cameraTPSEntity->AddComponent<TransformComponent>(TransformComponent({-35.0f, 0.0f, 35.5f}));
      cameraTPSEntity->AddComponent<CameraComponent>(
        CameraComponent(fieldOfView, aspectRatio, nearPlane, farPlane, cameraTPSEntity->GetId()));
      cameraFPSEntity = world.CreateEntity();
      cameraFPSEntity->AddComponent<TransformComponent>(TransformComponent({-35.0f, 0.0f, 35.5f}));
      cameraFPSEntity->AddComponent<CameraComponent>(
        CameraComponent(fieldOfView, aspectRatio, nearPlane, farPlane, cameraFPSEntity->GetId()));

      if (auto* cameraSystem = world.GetSystem<CameraSystem>()) {
        cameraSystem->SetActiveCamera(cameraTPSEntity->GetId());
      }

      //PL 1 - Entrée du tunnel
      {
        auto pointLightEntity = world.CreateEntity();


        XMFLOAT3           position{-22.5f, -20.5f, -24.f};
        TransformComponent transform(position);
        pointLightEntity->AddComponent<TransformComponent>(transform);
        pointLightEntity->AddComponent<LightComponent>(
          LightType::Point,
          XMFLOAT3(0.0, 0.8, 1.0), 
          100.0f,
          XMFLOAT3(-0.5f, -1.0f, -0.5f),
          15.0f
          
        );
      }
      //PL 2 - Dans le tunnel
      {
        auto pointLightEntity = world.CreateEntity();


        XMFLOAT3           position{-23.0f, -20.5f, 4.5f};
        TransformComponent transform(position);
        pointLightEntity->AddComponent<TransformComponent>(transform);
        pointLightEntity->AddComponent<LightComponent>(
          LightType::Point,
          XMFLOAT3(0.2, 0.3, 0.8), 
          60.0f,
          XMFLOAT3(-0.5f, -1.0f, -0.5f),
          15.0f

        );
      }
      //PL 3 - Dans le tunnel au niveau du checkpoint
      {
        auto pointLightEntity = world.CreateEntity();


        XMFLOAT3           position{-16.3f, -20.5f, 35.f};
        TransformComponent transform(position);
        pointLightEntity->AddComponent<TransformComponent>(transform);
        pointLightEntity->AddComponent<LightComponent>(
          LightType::Point,
          XMFLOAT3(0.31, 0.05, 0.10), 
          30.0f,
          XMFLOAT3(-0.5f, -1.0f, -0.5f),
          15.0f
      
        );
      }

      //PL 4 - Sortie du tunnel
      {
        auto pointLightEntity = world.CreateEntity();


        XMFLOAT3           position{-16.9f, -20.5f, 62.7f};
        TransformComponent transform(position);
        pointLightEntity->AddComponent<TransformComponent>(transform);
        pointLightEntity->AddComponent<LightComponent>(
          LightType::Point,
          XMFLOAT3(0.5 , 0.0 ,0.5), 
          10.0f,
          XMFLOAT3(-0.5f, -1.0f, -0.5f),
          15.0f
       
        );
      }

      //PL 5 - Sous le crâne
      {
        auto pointLightEntity = world.CreateEntity();


        XMFLOAT3           position{-105.f, -15.0f, 16.0f};
        TransformComponent transform(position);
        pointLightEntity->AddComponent<TransformComponent>(transform);
        pointLightEntity->AddComponent<LightComponent>(
          LightType::Point,
          XMFLOAT3(1, 1, 1), 
          15.0f,
          XMFLOAT3(0.f, -1.0f, 0.5f),
          10.0f
        );
      }

      //PL 6 - Dans le tunnel en haut
      {
        auto pointLightEntity = world.CreateEntity();


        XMFLOAT3           position{-15.2f, 5.2f, 17.5f};
        TransformComponent transform(position);
        pointLightEntity->AddComponent<TransformComponent>(transform);
        pointLightEntity->AddComponent<LightComponent>(
          LightType::Point,
          XMFLOAT3(0.98, 0.82, 0.25), 
          10.0f,
          XMFLOAT3(-0.5f, -1.0f, -0.5f),
          15.0f
        );
      }


      
      //PL 7 - Lave 1
   
      {
        auto pointLightEntity = world.CreateEntity();


        XMFLOAT3           position{ -64.0f, -20.5f, -12.4f };
        TransformComponent transform(position);
        pointLightEntity->AddComponent<TransformComponent>(transform);
        pointLightEntity->AddComponent<LightComponent>(
          LightType::Point,
          XMFLOAT3(1.0, 0.5, 0.0), 
          50.0f,
          XMFLOAT3(-0.5f, -1.0f, -0.5f),
          15.0f
        );


        //PL 8 - Lave 2
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -77.0f, -20.5f, -11.9f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0.5, 0.0), 
            50.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            15.0f
          );
        }

        ///PL 9 - Lave 4
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -90.0f, -20.5f, -11.9f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0.5, 0.0), 
            50.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            15.0f
         
          );
        }

        //PL 10 - Lave 5
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -103.3f, -20.5f, -11.6f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0.5, 0.0),
            50.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            15.0f
          );
        }

        //PL 11 - Lave 6
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -134.8f, -20.5f, -13.0f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0.5, 0.0),
            50.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            15.0f
          );
        }

        //PL 12 - Lave 7
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -121.9f, -20.50f, -11.0f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0.5, 0.0), 
            50.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            15.0f
          );
        }

        //PL 13 - Lave 8
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -56.f, -16.0f, 26.0f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0.5, 0.0), 
            10.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            10.0f
          );
        }

        //PL 14 - Bougie Violette 1
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -51.9f, -20.5f, 81.0f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(0.5, 0.0, 0.5), 
            10.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            10.0f
          );
        }

    
        //PL 15 - Bougie Violette 2
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -79.7f, -20.5f, 69.0f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(0.5, 0.0, 0.5), 
            20.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            10.0f
          );
        }

        //PL 16 - Bougie Violette 3
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -118.3f, -20.5f, 77.6f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(0.5, 0.0, 0.5), 
            20.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            10.0f

          );
        }

        //PL 17 - Bougie Violette 4
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -105.0f, -20.5f, 59.0f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(0.5, 0.0, 0.5), 
            20.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            10.0f

          );
        }

        //PL 18 - Bougie Violette 5
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -120.3f, -15.6f, 30.5f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(0.5, 0.0, 0.5), 
            20.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            10.0f

          );
        }
        
        //PL 19 - Bougie Violette 6
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -54.3f, -20.5f, 45.4f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(0.5, 0.0, 0.5), 
            20.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            10.0f
          );
        }

        //PL 20 - Bougie Orange 1
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -43.8f, -20.5f, -25.4f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0.5, 0.0), 
            20.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            10.0f
          );
        }

        //PL 21 - Bougie Orange 2
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -95.2f, -20.5f, -39.2f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0.5, 0.0), 
            20.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            10.0f
          );
        }

        //PL 22 - Bougie Orange 3
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{  -127.0f, -20.5f, -28.3f  };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0.5, 0.0), 
            20.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            10.0f
          );
        }

        //PL 23 - Bougie Orange 4
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -81.3f, -13.0f, 24.0f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0., 0.0), 
            20.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            5.0f
          );
        }

        //PL 24 - Lumière de la montée - Gauche - Bas
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -81.3f, -13.0f, 9.0f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0., 0.0), 
            20.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            5.0f

          );
        }

        //PL 25 - Lumière de la montée - Gauche - Haut
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -60.7f, -7.0f, 9.0f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0., 0.0),
            20.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            5.0f
          );
        }

        //PL 26 - Lumière de la montée - Droite - Haut
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -60.7f, -7.0f, 24.0f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0., 0.0), 
            20.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            5.0f
          );
        }

        //PL 27 - Lumière de la montée - Droite - Bas
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -105.2f, -4.45f, 12.1f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0., 0.0), 
            20.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            3.0f
          );
        }

        //PL28 - Oeil gauche du crâne
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -105.2f, -4.46f, 23.f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0., 0.0), 
            20.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            3.0f
          );
        }

        //PL29 - Oeil droit du crâne
        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ -120.3f, -20.5f, 13.9f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 0.5, 0.0), 
            20.0f,
            XMFLOAT3(-0.5f, -1.0f, -0.5f),
            10.0f
          );
        }

        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ 30.028f, -20.5f, 12.502f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 1.0, 1.0),
            15.0f,
            XMFLOAT3(-0.0f, -1.0f, -0.0f),
            50.0f
          );
        }

        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ 81.0f, -20.5f,-34.0f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 1.0, 1.0),
            15.0f,
            XMFLOAT3(-0.0f, -1.0f, -0.0f),
            30.0f
          );
        }

        {
          auto pointLightEntity = world.CreateEntity();


          XMFLOAT3           position{ 75.028f, -20.5f, 12.502f };
          TransformComponent transform(position);
          pointLightEntity->AddComponent<TransformComponent>(transform);
          pointLightEntity->AddComponent<LightComponent>(
            LightType::Point,
            XMFLOAT3(1.0, 1.0, 1.0),
            15.0f,
            XMFLOAT3(-0.0f, -1.0f, -0.0f),
            30.0f
          );
        }

        

      }


    }

  private:
    std::shared_ptr<Entity> freeCameraEntity;
    std::shared_ptr<Entity> cameraTPSEntity;
    std::shared_ptr<Entity> cameraFPSEntity;
  };
}
