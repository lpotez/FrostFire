#pragma once
#include "BaseScene.h"
#include "Scene.h"
#include "MeshFactory.h"
#include "ECS/components/mesh/MeshComponent.h"
#include "ECS/components/rendering/PBRRenderer.h"
#include "FreeCameraScript.h"
#include "ECS/components/LightComponent.h"

namespace FrostFireEngine
{
  class TestScene : public BaseScene {
  public:
    void Initialize(DispositifD3D11 *pDevice) override
    {
      BaseScene::Initialize(pDevice);
      World &world = GetWorld();

      auto &renderingSystem = *world.GetSystem<RenderingSystem>();
      TextureManager &textureManager = TextureManager::GetInstance();
      std::wstring skyboxPath = L"../EngineTest/Assets/sky.dds";
      auto skyboxTexture = textureManager.GetNewTexture(skyboxPath, pDevice);
      renderingSystem.SetSkyboxTexture(skyboxTexture);

      std::wstring irradiancePath = L"Assets\\objects\\test\\skybox2.dds";
      auto irradianceTexture = textureManager.GetNewTexture(irradiancePath, pDevice);

      std::wstring prefilteredEnvPath = L"Assets\\objects\\test\\skybox2IR.dds";
      auto prefilteredEnvTexture = textureManager.GetNewTexture(prefilteredEnvPath, pDevice);

      std::wstring brdfLUTPath = L"Assets\\objects\\test\\ibl_brdf_lut.png";
      auto brdfLUTTexture = textureManager.GetNewTexture(brdfLUTPath, pDevice);

      const int gridSize = 5;
      const float spacing = 3.0f;
      const float sphereRadius = 1.0f;

      for (int x = 0; x < gridSize; ++x) {
        for (int z = 0; z < gridSize; ++z) {
          auto sphereEntity = world.CreateEntity();

          float posX = (x - gridSize / 2.0f) * spacing;
          float posZ = (z - gridSize / 2.0f) * spacing;

          auto &transform = sphereEntity->AddComponent<TransformComponent>();
          transform.SetPosition({ posX, 0.0f, posZ });

          auto &sphereMesh = sphereEntity->AddComponent<MeshComponent>();
          sphereMesh.SetMesh(MeshFactory::CreateSphere(pDevice->GetD3DDevice(), sphereRadius));

          auto &pbrRenderer = sphereEntity->AddComponent<PBRRenderer>(pDevice->GetD3DDevice());

          float metallic = static_cast<float>(x) / (gridSize - 1);
          float roughness = static_cast<float>(z) / (gridSize - 1);

          pbrRenderer.SetBaseColor({ 0.7f, 0.7f, 0.7f, 1.0f });
          pbrRenderer.SetMetallic(metallic);
          pbrRenderer.SetRoughness(roughness);
          pbrRenderer.SetAmbientOcclusion(1.0f);
          pbrRenderer.SetIrradianceMap(irradianceTexture);
          pbrRenderer.SetPrefilteredEnvMap(prefilteredEnvTexture);
          pbrRenderer.SetBRDFLUT(brdfLUTTexture);
        }
      }

      auto planeEntity = world.CreateEntity();
      auto &planeTransform = planeEntity->AddComponent<TransformComponent>();
      planeTransform.SetPosition({ 0.0f, -2.0f, 0.0f });
      auto &planeMesh = planeEntity->AddComponent<MeshComponent>();
      planeMesh.SetMesh(MeshFactory::CreatePlane(pDevice->GetD3DDevice(), 20.0f, 20.0f));
      auto &planePBR = planeEntity->AddComponent<PBRRenderer>(pDevice->GetD3DDevice());
      planePBR.SetBaseColor({ 0.2f, 0.2f, 0.2f, 1.0f });
      planePBR.SetMetallic(0.0f);
      planePBR.SetRoughness(0.9f);
      planePBR.SetIrradianceMap(irradianceTexture);
      planePBR.SetPrefilteredEnvMap(prefilteredEnvTexture);
      planePBR.SetBRDFLUT(brdfLUTTexture);

      auto mainLight = world.CreateEntity();
      mainLight->AddComponent<LightComponent>(
        LightType::Directional,
        XMFLOAT3(1.0f, 1.0f, 1.0f),
        1.0f,
        XMFLOAT3(-0.5f, -1.0f, -0.5f)
      );

      CreatePointLight(world, { 5.0f, 3.0f, 5.0f }, { 1.0f, 0.8f, 0.6f });
      CreatePointLight(world, { -5.0f, 3.0f, -5.0f }, { 0.6f, 0.8f, 1.0f });

      World::GetInstance().BuildOctree();
    }

  protected:
    void SetupCamera() override
    {
      World &world = GetWorld();

      constexpr float fieldOfView = XM_PI / 4;
      constexpr float aspectRatio = 16.0f / 9.0f;
      constexpr float nearPlane = 0.1f;
      constexpr float farPlane = 1000.0f;

      auto freeCameraEntity = world.CreateEntity();
      auto &transform = freeCameraEntity->AddComponent<TransformComponent>();
      transform.SetPosition({ 0.0f, 5.0f, -15.0f });

      freeCameraEntity->AddComponent<CameraComponent>(
        CameraComponent(fieldOfView, aspectRatio, nearPlane, farPlane, freeCameraEntity->GetId())
      );
      freeCameraEntity->AddComponent<FreeCameraScript>();

      if (auto *cameraSystem = world.GetSystem<CameraSystem>()) {
        cameraSystem->SetActiveCamera(freeCameraEntity->GetId());
      }
    }

  private:
    void CreatePointLight(World &world, const XMFLOAT3 &position, const XMFLOAT3 &color)
    {
      auto pointLight = world.CreateEntity();
      auto &transform = pointLight->AddComponent<TransformComponent>();
      transform.SetPosition(position);

      pointLight->AddComponent<LightComponent>(
        LightType::Point,
        color,
        2.0f,
        XMFLOAT3(0.0f, -1.0f, 0.0f),
        10.0f
      );
    }
  };
}
