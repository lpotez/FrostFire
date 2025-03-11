#pragma once
#include <fbxsdk.h>
#include <string>
#include <memory>
#include <DirectXMath.h>
#include <filesystem>
#include <map>
#include <fstream>
#include <vector>

#include "Engine/ECS/core/Entity.h"
#include "TextureManager.h"
#include "Mesh.h"
#include "Engine/ECS/core/World.h"
#include "ECS/components/mesh/MeshComponent.h"
#include "ECS/components/physics/ColliderComponent.h"
#include "ECS/components/physics/RigidBodyComponent.h"
#include "ECS/components/rendering/PBRRenderer.h"

namespace FrostFireEngine
{
  #undef min
  #undef max
  class FBXEntityBuilder {
  public:
    struct BuildSettings {
      std::string                 fbxPath;
      XMFLOAT3                    basePosition{0.0f, 0.0f, 0.0f};
      XMFLOAT4                    baseRotation{0.0f, 0.0f, 0.0f, 1.0f};
      XMFLOAT3                    baseScale{1.0f, 1.0f, 1.0f};
      float                       scaleFactor = 1.0f;
      bool                        generateColliders = false;
      bool                        addRigidbody = false;
      RigidBodyComponent::Type    rigidBodyType = RigidBodyComponent::Type::Static;
      ColliderComponent::MeshType colliderMeshType = ColliderComponent::MeshType::Convex;
      bool                        optimizeMeshes = true;
      bool                        flipUVs = false;
      bool                        importMaterials = true;
      bool                        importTextures = true;
    };


    struct BuildResult {
      bool                    success = false;
      std::shared_ptr<Entity> rootEntity;
      std::string             errorMessage;
    };

    FBXEntityBuilder(DispositifD3D11* pDispositif)
      : m_pDispositif(pDispositif), m_textureManager(&TextureManager::GetInstance())
    {
      InitializeFBXSDK();
    }

    ~FBXEntityBuilder()
    {
      CleanupFBXSDK();
    }

    BuildResult BuildFromFile(const BuildSettings& settings)
    {
      BuildResult result;

      // Chemin du fichier de cache
      std::filesystem::path fbxFilePath(settings.fbxPath);
      std::filesystem::path cacheFilePath = fbxFilePath;
      cacheFilePath.replace_extension(".cache");

      if (exists(cacheFilePath)) {
        // Charger depuis le cache
        if (LoadFromCache(cacheFilePath.string(), result)) {
          result.success = true;
          return result;
        }
        else {
          result.errorMessage = "Échec du chargement depuis le cache";
          return result;
        }
      }
      else {
        // Charger depuis le fichier FBX
        FbxScene* scene = nullptr;
        if (!LoadFBXScene(settings.fbxPath, scene)) {
          result.errorMessage = "Échec du chargement de la scène FBX";
          return result;
        }

        const auto rootEntity = World::GetInstance().CreateEntity();

        FbxNode*         rootNode = scene->GetRootNode();
        const FbxAMatrix rootGlobalTransform = rootNode->EvaluateGlobalTransform();

        FbxVector4       rotationPivot = rootNode->GetRotationPivot(FbxNode::eSourcePivot);
        FbxVector4       scalingPivot = rootNode->GetScalingPivot(FbxNode::eSourcePivot);
        const FbxVector4 pivotPoint(
          (rotationPivot[0] + scalingPivot[0]) * 0.5,
          (rotationPivot[1] + scalingPivot[1]) * 0.5,
          (rotationPivot[2] + scalingPivot[2]) * 0.5,
          1.0
        );
        FbxVector4 globalPivot = rootGlobalTransform.MultT(pivotPoint);

        const XMFLOAT3 pivotPosition(
          static_cast<float>(globalPivot[0] * settings.scaleFactor),
          static_cast<float>(globalPivot[1] * settings.scaleFactor),
          static_cast<float>(globalPivot[2] * settings.scaleFactor)
        );

        rootEntity->AddComponent<TransformComponent>(
          XMFLOAT3(
            settings.basePosition.x + pivotPosition.x,
            settings.basePosition.y + pivotPosition.y,
            settings.basePosition.z + pivotPosition.z
          ),
          settings.baseRotation,
          settings.baseScale
        );

        ProcessNode(rootNode, rootEntity, settings, globalPivot);

        result.success = true;
        result.rootEntity = rootEntity;

        // Sauvegarder dans le cache
        SaveToCache(cacheFilePath.string(), rootEntity);

        return result;
      }
    }

  private:
    DispositifD3D11* m_pDispositif;
    TextureManager*  m_textureManager;
    FbxManager*      m_pFbxManager = nullptr;
    FbxIOSettings*   m_pIOSettings = nullptr;

    XMFLOAT3 m_minBounds{FLT_MAX, FLT_MAX, FLT_MAX};
    XMFLOAT3 m_maxBounds{-FLT_MAX, -FLT_MAX, -FLT_MAX};

    void InitializeFBXSDK()
    {
      m_pFbxManager = FbxManager::Create();
      m_pIOSettings = FbxIOSettings::Create(m_pFbxManager, IOSROOT);
      m_pFbxManager->SetIOSettings(m_pIOSettings);
    }

    void CleanupFBXSDK()
    {
      if (m_pIOSettings) {
        m_pIOSettings->Destroy();
        m_pIOSettings = nullptr;
      }
      if (m_pFbxManager) {
        m_pFbxManager->Destroy();
        m_pFbxManager = nullptr;
      }
    }

    bool LoadFBXScene(const std::string& filePath, FbxScene*& outScene) const
    {
      FbxImporter* importer = FbxImporter::Create(m_pFbxManager, "");
      outScene = FbxScene::Create(m_pFbxManager, "");

      const FbxAxisSystem directXAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityEven,
                                            FbxAxisSystem::eRightHanded);

      if (!importer->Initialize(filePath.c_str(), -1, m_pFbxManager->GetIOSettings())) {
        importer->Destroy();
        return false;
      }

      const bool result = importer->Import(outScene);
      importer->Destroy();

      directXAxisSystem.ConvertScene(outScene);

      if (result) {
        FbxGeometryConverter geometryConverter(m_pFbxManager);
        geometryConverter.Triangulate(outScene, true);
      }

      return result;
    }
    void ProcessNode(FbxNode*                       fbxNode,
                     const std::shared_ptr<Entity>& parentEntity,
                     const BuildSettings&           settings,
                     const FbxVector4&              globalPivot)
    {
      if (!fbxNode) return;

      const auto nodeEntity = World::GetInstance().CreateEntity();

      FbxAMatrix globalTransform = fbxNode->EvaluateGlobalTransform();

      FbxAMatrix pivotOffset;
      pivotOffset.SetT(globalPivot);
      globalTransform = globalTransform * pivotOffset.Inverse();

      FbxAMatrix parentGlobalTransform;
      if (fbxNode->GetParent()) {
        parentGlobalTransform = fbxNode->GetParent()->EvaluateGlobalTransform();
      }

      const FbxAMatrix localTransform = parentGlobalTransform.Inverse() * globalTransform;

      FbxVector4 translation = localTransform.GetT();
      FbxVector4 rotation = localTransform.GetR();
      FbxVector4 scaling = localTransform.GetS();

      const XMFLOAT4 rotationQuaternion(
        static_cast<float>(rotation[0]),
        static_cast<float>(rotation[1]),
        static_cast<float>(rotation[2]),
        static_cast<float>(rotation[3])
      );

      const XMFLOAT3 position(
        static_cast<float>(translation[0] * settings.scaleFactor),
        static_cast<float>(translation[1] * settings.scaleFactor),
        static_cast<float>(translation[2] * settings.scaleFactor)
      );

      const auto transform = TransformComponent(
        position,
        rotationQuaternion,
        XMFLOAT3(
          static_cast<float>(scaling[0]),
          static_cast<float>(scaling[1]),
          static_cast<float>(scaling[2])
        )
      );
      nodeEntity->AddComponent<TransformComponent>(std::move(transform));

      if (parentEntity) {
        if (const auto parentTransform = parentEntity->GetComponent<TransformComponent>()) {
          parentTransform->AddChild(nodeEntity->GetId());
        }
      }

      if (fbxNode->GetNodeAttribute() &&
        fbxNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
        ProcessMesh(fbxNode->GetMesh(), nodeEntity, settings, globalPivot);
      }

      for (int i = 0; i < fbxNode->GetChildCount(); ++i) {
        ProcessNode(fbxNode->GetChild(i), nodeEntity, settings, globalPivot);
      }
    }


    struct MaterialGroup {
      std::vector<Vertex>                      vertices;
      std::vector<uint32_t>                    indices;
      FbxSurfaceMaterial*                      material = nullptr;
      int                                      materialIndex = 0;
      std::map<std::tuple<int, int, int>, int> uniqueVertices;
      int                                      currentVertexIndex = 0;
    };

    void ProcessMesh(FbxMesh*                fbxMesh,
                     std::shared_ptr<Entity> entity,
                     const BuildSettings&    settings,
                     const FbxVector4&       globalPivot)
    {
      if (!fbxMesh) return;

      fbxMesh->GenerateNormals();
      fbxMesh->GenerateTangentsData();
      FbxLayer*              layer = fbxMesh->GetLayer(0);
      FbxLayerElementNormal* layerElementNormal = layer->GetNormals();

      // Récupération des matériaux
      std::vector<MaterialGroup> materialGroups;
      FbxNode*                   node = fbxMesh->GetNode();
      int                        materialCount = node->GetMaterialCount();

      // Si aucun matériau, créer un groupe avec matériau par défaut
      if (materialCount == 0) {
        MaterialGroup defaultGroup;
        defaultGroup.material = nullptr;
        defaultGroup.materialIndex = 0;
        materialGroups.push_back(defaultGroup);
      }
      else {
        // Créer un groupe pour chaque matériau
        for (int i = 0; i < materialCount; i++) {
          MaterialGroup group;
          group.material = node->GetMaterial(i);
          group.materialIndex = i;
          materialGroups.push_back(group);
        }
      }

      // Préparation des matrices de transformation
      FbxAMatrix meshGlobalTransform = fbxMesh->GetNode()->EvaluateGlobalTransform();
      FbxAMatrix pivotOffset;
      pivotOffset.SetT(globalPivot);
      meshGlobalTransform = meshGlobalTransform * pivotOffset.Inverse();

      FbxAMatrix normalMatrix = meshGlobalTransform.Inverse().Transpose();
      normalMatrix.SetT(FbxVector4(0, 0, 0, 1));

      // Récupération des éléments de mapping des matériaux
      FbxGeometryElementMaterial* materialElement = fbxMesh->GetElementMaterial();
      FbxGeometryElementUV*       uvElement = fbxMesh->GetElementUV(0);
      FbxVector4*                 controlPoints = fbxMesh->GetControlPoints();

      // Création de la carte UV si nécessaire
      std::vector<std::tuple<int, int, FbxVector2>> uvMap;
      if (uvElement) {
        uvMap.resize(fbxMesh->GetPolygonVertexCount());
        int vertexCount = 0;
        for (int polyIndex = 0; polyIndex < fbxMesh->GetPolygonCount(); polyIndex++) {
          for (int vertexIndex = 0; vertexIndex < 3; vertexIndex++) {
            int        ctrlPointIndex = fbxMesh->GetPolygonVertex(polyIndex, vertexIndex);
            int        uvIndex = fbxMesh->GetTextureUVIndex(polyIndex, vertexIndex);
            FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);
            uvMap[vertexCount++] = std::make_tuple(ctrlPointIndex, uvIndex, uv);
          }
        }
      }

      // Traitement des polygones
      int vertexCount = 0;
      for (int polyIndex = 0; polyIndex < fbxMesh->GetPolygonCount(); polyIndex++) {
        // Détermination du matériau pour ce polygone
        int materialIndex = 0;
        if (materialElement) {
          switch (materialElement->GetMappingMode()) {
            case FbxGeometryElement::eByPolygon:
              materialIndex = materialElement->GetIndexArray().GetAt(polyIndex);
              break;
            case FbxGeometryElement::eAllSame:
              materialIndex = materialElement->GetIndexArray().GetAt(0);
              break;
            default:
              // En cas de mapping non supporté, utiliser le premier matériau
              materialIndex = 0;
              break;
          }
        }

        // Vérification de la validité de l'index du matériau
        if (materialIndex >= static_cast<int>(materialGroups.size())) {
          materialIndex = 0;
        }

        MaterialGroup& currentGroup = materialGroups[materialIndex];

        // Traitement des sommets du polygone
        for (int v = 0; v < 3; v++) {
          int ctrlPointIndex = fbxMesh->GetPolygonVertex(polyIndex, v);

          // Récupération de la normale
          FbxVector4 normal;
          if (layerElementNormal) {
            if (layerElementNormal->GetMappingMode() == FbxLayerElement::eByControlPoint) {
              switch (layerElementNormal->GetReferenceMode()) {
                case FbxLayerElement::eDirect:
                  normal = layerElementNormal->GetDirectArray().GetAt(ctrlPointIndex);
                  break;
                case FbxLayerElement::eIndexToDirect:
                  {
                    int index = layerElementNormal->GetIndexArray().GetAt(ctrlPointIndex);
                    normal = layerElementNormal->GetDirectArray().GetAt(index);
                  }
                  break;
              }
            }
            else if (layerElementNormal->GetMappingMode() == FbxLayerElement::eByPolygonVertex) {
              switch (layerElementNormal->GetReferenceMode()) {
                case FbxLayerElement::eDirect:
                  normal = layerElementNormal->GetDirectArray().GetAt(vertexCount);
                  break;
                case FbxLayerElement::eIndexToDirect:
                  {
                    int index = layerElementNormal->GetIndexArray().GetAt(vertexCount);
                    normal = layerElementNormal->GetDirectArray().GetAt(index);
                  }
                  break;
              }
            }
          }

          ProcessVertex(currentGroup, ctrlPointIndex, normal, normalMatrix, meshGlobalTransform,
                        settings, controlPoints, uvElement, uvMap, vertexCount++, fbxMesh);
        }
      }

      // Création des entités pour chaque groupe de matériau
      for (const auto& group : materialGroups) {
        if (group.vertices.empty()) continue;

        auto childEntity = World::GetInstance().CreateEntity();
        auto mesh = std::make_shared<Mesh>(m_pDispositif->GetD3DDevice(), group.vertices,
                                           group.indices);


        childEntity->AddComponent<TransformComponent>();
        auto& renderer = childEntity->AddComponent<PBRRenderer>(
          m_pDispositif->GetD3DDevice());
        auto& meshComponent = childEntity->AddComponent<MeshComponent>();
        meshComponent.SetMesh(mesh);

        // Configuration du matériau
        ProcessMaterial(group.material, renderer);


        // Configuration des textures
        if (settings.importTextures) {
          ProcessTextures(group.material, renderer);
        }


        if (settings.generateColliders) {
          auto& collider = childEntity->AddComponent<ColliderComponent>(
            settings.colliderMeshType == ColliderComponent::MeshType::Convex
              ? ColliderComponent::Type::ConvexMesh
              : ColliderComponent::Type::TriangleMesh
          );
          collider.SetMeshType(settings.colliderMeshType);
          collider.Initialize(XMFLOAT3(1.0f, 1.0f, 1.0f));

          if (settings.addRigidbody) {
            auto& rigidBody = childEntity->AddComponent<RigidBodyComponent>(settings.rigidBodyType);
            rigidBody.Initialize();
          }
        }

        auto parentTransform = entity->GetComponent<TransformComponent>();
        parentTransform->AddChild(childEntity->GetId());
      }

      // Mise à jour des limites du maillage global
      for (const auto& group : materialGroups) {
        for (const auto& vertex : group.vertices) {
          const XMFLOAT3& pos = vertex.GetPosition();
          m_minBounds.x = std::min(m_minBounds.x, pos.x);
          m_minBounds.y = std::min(m_minBounds.y, pos.y);
          m_minBounds.z = std::min(m_minBounds.z, pos.z);

          m_maxBounds.x = std::max(m_maxBounds.x, pos.x);
          m_maxBounds.y = std::max(m_maxBounds.y, pos.y);
          m_maxBounds.z = std::max(m_maxBounds.z, pos.z);
        }
      }

      // Réinitialisation de la transformation de l'entité parente
      auto transform = entity->GetComponent<TransformComponent>();
      transform->SetPosition(XMFLOAT3(0, 0, 0));
      transform->SetRotation(XMFLOAT4(0, 0, 0, 1));
      transform->SetScale(XMFLOAT3(1, 1, 1));
    }

    void ProcessVertex(MaterialGroup&                                       group,
                       int                                                  ctrlPointIndex,
                       const FbxVector4&                                    normal,
                       const FbxAMatrix&                                    normalMatrix,
                       const FbxAMatrix&                                    meshTransform,
                       const BuildSettings&                                 settings,
                       FbxVector4*                                          controlPoints,
                       FbxGeometryElementUV*                                uvElement,
                       const std::vector<std::tuple<int, int, FbxVector2>>& uvMap,
                       int                                                  vertexIndex,
                       FbxMesh*                                             fbxMesh)
    {
      // Création d'une clé unique pour chaque combinaison de position et UV
      FbxVector2 currentUV(0, 0);
      if (uvElement) {
        if (uvElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
          int uvIndex;
          if (uvElement->GetReferenceMode() == FbxGeometryElement::eDirect) {
            uvIndex = vertexIndex;
          }
          else // eIndexToDirect
          {
            uvIndex = uvElement->GetIndexArray().GetAt(vertexIndex);
          }
          currentUV = uvElement->GetDirectArray().GetAt(uvIndex);
        }
        else if (uvElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
          int uvIndex;
          if (uvElement->GetReferenceMode() == FbxGeometryElement::eDirect) {
            uvIndex = ctrlPointIndex;
          }
          else // eIndexToDirect
          {
            uvIndex = uvElement->GetIndexArray().GetAt(ctrlPointIndex);
          }
          currentUV = uvElement->GetDirectArray().GetAt(uvIndex);
        }
      }

      // Création d'une clé unique qui prend en compte à la fois la position et les UVs
      const auto vertexKey = std::make_tuple(
        ctrlPointIndex,
        static_cast<int>(currentUV[0] * 1000),
        static_cast<int>(currentUV[1] * 1000)
      );
      const auto it = group.uniqueVertices.find(vertexKey);

      if (it == group.uniqueVertices.end()) {
        Vertex     vertex;
        FbxVector4 position = meshTransform.MultT(controlPoints[ctrlPointIndex]);

        vertex.SetPosition(XMFLOAT3(
          static_cast<float>(position[0] * settings.scaleFactor),
          static_cast<float>(position[1] * settings.scaleFactor),
          static_cast<float>(position[2] * settings.scaleFactor)
        ));

        // Configuration de la normale...
        FbxVector4 transformedNormal = normalMatrix.MultT(normal);
        transformedNormal.Normalize();
        vertex.SetNormal(XMFLOAT3(
          static_cast<float>(transformedNormal[0]),
          static_cast<float>(transformedNormal[1]),
          static_cast<float>(transformedNormal[2])
        ));

        // Configuration des UVs
        if (uvElement) {
          vertex.SetTexCoord(XMFLOAT2(
            static_cast<float>(currentUV[0]),
            settings.flipUVs
              ? static_cast<float>(1.0 - currentUV[1])
              : static_cast<float>(currentUV[1])
          ));
        }
        else {
          vertex.SetTexCoord(XMFLOAT2(0.0f, 0.0f));
        }

        // Configuration des tangentes...
        FbxGeometryElementTangent* tangentElement = fbxMesh->GetElementTangent(0);
        if (tangentElement) {
          FbxVector4 tangent;
          if (tangentElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
            int tangentIndex = (tangentElement->GetReferenceMode() == FbxGeometryElement::eDirect)
                                 ? ctrlPointIndex
                                 : tangentElement->GetIndexArray().GetAt(ctrlPointIndex);
            tangent = tangentElement->GetDirectArray().GetAt(tangentIndex);
          }
          else if (tangentElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
            int tangentIndex = (tangentElement->GetReferenceMode() == FbxGeometryElement::eDirect)
                                 ? vertexIndex
                                 : tangentElement->GetIndexArray().GetAt(vertexIndex);
            tangent = tangentElement->GetDirectArray().GetAt(tangentIndex);
          }

          FbxVector4 transformedTangent = normalMatrix.MultT(tangent);
          transformedTangent.Normalize();

          vertex.SetTangent(XMFLOAT3(
            static_cast<float>(transformedTangent[0]),
            static_cast<float>(transformedTangent[1]),
            static_cast<float>(transformedTangent[2])
          ));
        }
        else {
          vertex.SetTangent(XMFLOAT3(1.0f, 0.0f, 0.0f));
        }

        group.vertices.push_back(vertex);
        group.uniqueVertices[vertexKey] = group.currentVertexIndex;
        group.indices.push_back(group.currentVertexIndex);
        group.currentVertexIndex++;
      }
      else {
        group.indices.push_back(it->second);
      }
    }


    static void ProcessMaterial(FbxSurfaceMaterial* fbxMaterial, PBRRenderer& renderer)
    {
      if (!fbxMaterial) return;


      XMFLOAT4 baseColor = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
      float    metallic = 0.0f;
      float    roughness = 0.5f;
      float    ao = 1.0f;

      if (FbxSurfaceLambert* lambert = FbxCast<FbxSurfaceLambert>(fbxMaterial)) {
        // Diffuse = baseColor
        {
          FbxDouble3 lDiffuse = lambert->Diffuse;
          FbxDouble  lDiffuseFactor = lambert->DiffuseFactor;
          baseColor = XMFLOAT4(
            static_cast<float>(lDiffuse[0] * lDiffuseFactor),
            static_cast<float>(lDiffuse[1] * lDiffuseFactor),
            static_cast<float>(lDiffuse[2] * lDiffuseFactor),
            1.0f
          );
        }

        FbxSurfacePhong* phong = FbxCast<FbxSurfacePhong>(fbxMaterial);
        if (phong) {
          // Récupérer la spéculaire et le shininess
          FbxDouble3 lSpecular = phong->Specular;
          FbxDouble  lSpecularFactor = phong->SpecularFactor;
          FbxDouble  lShininess = phong->Shininess;

          // Convertir specular en metallic/roughness (Heuristique)
          // Si le modèle a un specular élevé, on considère qu'il est métallique.
          float specularAvg = static_cast<float>((lSpecular[0] + lSpecular[1] + lSpecular[2]) /
            3.0f);
          specularAvg = specularAvg * static_cast<float>(lSpecularFactor);

          // Si specular moyen > 0.5, on considère que c'est métal
          if (specularAvg > 0.5f) {
            metallic = 1.0f;
          }
          else {
            metallic = 0.0f;
          }

          // Convertir le shininess en roughness
          // shininess élevé => surface lisse => roughness faible
          // roughness = sqrt(2/(N+2)) est une formule courante, où N=shininess
          // On s'assure de clipper entre [0.04,1.0] pour éviter des valeurs extrêmes
          float N = static_cast<float>(lShininess);
          float computedRoughness = sqrtf(2.0f / (N + 2.0f));
          computedRoughness = std::max(0.04f, std::min(computedRoughness, 1.0f));
          roughness = computedRoughness;
        }
        else {
          // Pas de phong, donc matériau purement lambertien:
          // On garde metallic bas et roughness moyen.
          metallic = 0.0f;
          roughness = 0.9f;
        }
      }

      // TODO : AO fournir par FBX dans ce code
      ao = 1.0f;

      renderer.SetBaseColor(baseColor);
      renderer.SetMetallic(metallic);
      renderer.SetRoughness(roughness);
      renderer.SetAmbientOcclusion(ao);
    }

    void ProcessTextures(const FbxSurfaceMaterial* fbxMaterial, PBRRenderer& renderer)
    {
      if (!fbxMaterial) return;

      const FbxProperty albedoProperty = fbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
      if (albedoProperty.IsValid()) {
        const int textureCount = albedoProperty.GetSrcObjectCount<FbxFileTexture>();
        if (textureCount > 0) {
          if (const FbxFileTexture* texture = albedoProperty.GetSrcObject<FbxFileTexture>(0)) {
            const std::filesystem::path texturePath = texture->GetFileName();
            const std::wstring          widePath = texturePath.wstring();
            if (auto* albedoTexture = m_textureManager->GetNewTexture(
              widePath.c_str(), m_pDispositif)) {
              renderer.SetAlbedoTexture(albedoTexture);
            }
          }
        }
      }

      FbxProperty normalProperty = fbxMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
      if (!normalProperty.IsValid()) {
        normalProperty = fbxMaterial->FindProperty(FbxSurfaceMaterial::sBump);
      }
      if (normalProperty.IsValid()) {
        const int textureCount = normalProperty.GetSrcObjectCount<FbxFileTexture>();
        if (textureCount > 0) {
          if (const FbxFileTexture* texture = normalProperty.GetSrcObject<FbxFileTexture>(0)) {
            const std::filesystem::path texturePath = texture->GetFileName();
            const std::wstring          widePath = texturePath.wstring();
            if (auto* normalTexture = m_textureManager->GetNewTexture(
              widePath.c_str(), m_pDispositif)) {
              renderer.SetNormalMap(normalTexture);
            }
          }
        }
      }

      FbxProperty occlusionProperty = fbxMaterial->FindProperty("occlusionMap");
      if (!occlusionProperty.IsValid()) {
        occlusionProperty = fbxMaterial->FindProperty("ambientOcclusionMap");
      }
      if (occlusionProperty.IsValid()) {
        const int textureCount = occlusionProperty.GetSrcObjectCount<FbxFileTexture>();
        if (textureCount > 0) {
          if (const FbxFileTexture* texture = occlusionProperty.GetSrcObject<FbxFileTexture>(0)) {
            const std::filesystem::path texturePath = texture->GetFileName();
            const std::wstring          widePath = texturePath.wstring();
            if (auto* occlusionTexture = m_textureManager->GetNewTexture(
              widePath.c_str(), m_pDispositif)) {
              renderer.SetAOMap(occlusionTexture);
            }
          }
        }
      }

      FbxProperty metallicProperty = fbxMaterial->FindProperty("metallicMap");
      if (metallicProperty.IsValid()) {
        const int textureCount = metallicProperty.GetSrcObjectCount<FbxFileTexture>();
        if (textureCount > 0) {
          if (const FbxFileTexture* texture = metallicProperty.GetSrcObject<FbxFileTexture>(0)) {
            const std::filesystem::path texturePath = texture->GetFileName();
            const std::wstring          widePath = texturePath.wstring();
            if (auto* metallicTexture = m_textureManager->GetNewTexture(
              widePath.c_str(), m_pDispositif)) {
              renderer.SetMetallicRoughnessMap(metallicTexture);
            }
          }
        }
      }
      /*
      FbxProperty roughnessProperty = fbxMaterial->FindProperty("roughnessMap");
      if (roughnessProperty.IsValid()) {
        const int textureCount = roughnessProperty.GetSrcObjectCount<FbxFileTexture>();
        if (textureCount > 0) {
          if (const FbxFileTexture* texture = roughnessProperty.GetSrcObject<FbxFileTexture>(0)) {
            const std::filesystem::path texturePath = texture->GetFileName();
            const std::wstring          widePath = texturePath.wstring();
            if (auto* roughnessTexture = m_textureManager->GetNewTexture(
              widePath.c_str(), m_pDispositif)) {
              renderer.SetRoughnessMap(roughnessTexture);
            }
          }
        }
      }
      */

    }

    // Fonctions de sérialisation
    bool SaveToCache(const std::string& cacheFilePath, const std::shared_ptr<Entity>& rootEntity)
    {
      std::ofstream ofs(cacheFilePath, std::ios::binary);
      if (!ofs) return false;

      return SerializeEntity(ofs, rootEntity);
    }

    bool LoadFromCache(const std::string& cacheFilePath, BuildResult& result)
    {
      std::ifstream ifs(cacheFilePath, std::ios::binary);
      if (!ifs) return false;

      auto rootEntity = DeserializeEntity(ifs);
      if (rootEntity) {
        result.rootEntity = rootEntity;
        return true;
      }
      else {
        return false;
      }
    }

    bool SerializeEntity(std::ofstream& ofs, const std::shared_ptr<Entity>& entity)
    {
      // Sérialisation de TransformComponent
      if (auto transform = entity->GetComponent<TransformComponent>()) {
        ofs.put(1); // Possède TransformComponent
        SerializeTransformComponent(ofs, *transform);
      }
      else {
        ofs.put(0); // Ne possède pas TransformComponent
      }

      // Sérialisation de MeshComponent
      if (auto meshComponent = entity->GetComponent<MeshComponent>()) {
        ofs.put(1); // Possède MeshComponent
        SerializeMeshComponent(ofs, *meshComponent);
      }
      else {
        ofs.put(0); // Ne possède pas BlinnPhongRenderer
      }

      // Sérialisation de PBRRenderer
      if (auto renderer = entity->GetComponent<PBRRenderer>()) {
        ofs.put(1);
        SerializePBRRenderer(ofs, *renderer);
      }
      else {
        ofs.put(0);
      }


      // Sérialisation de ColliderComponent
      if (auto collider = entity->GetComponent<ColliderComponent>()) {
        ofs.put(1); // Possède ColliderComponent
        ColliderComponent::Type colliderType = collider->GetType();
        ofs.write(reinterpret_cast<const char*>(&colliderType), sizeof(colliderType));
        collider->Serialize(ofs);
      }
      else {
        ofs.put(0); // Ne possède pas ColliderComponent
      }

      // Sérialisation de RigidBodyComponent
      if (auto rigidBody = entity->GetComponent<RigidBodyComponent>()) {
        ofs.put(1); // Possède RigidBodyComponent
        RigidBodyComponent::Type type = rigidBody->GetType();
        ofs.write(reinterpret_cast<const char*>(&type), sizeof(type));
      }
      else {
        ofs.put(0); // Ne possède pas RigidBodyComponent
      }


      // Sérialisation des enfants
      const auto& transform = entity->GetComponent<TransformComponent>();
      if (transform) {
        const auto& children = transform->GetChildren();
        uint32_t    numChildren = static_cast<uint32_t>(children.size());
        ofs.write(reinterpret_cast<const char*>(&numChildren), sizeof(numChildren));

        for (const auto& childId : children) {
          auto childEntity = World::GetInstance().GetEntity(childId);
          if (childEntity) {
            SerializeEntity(ofs, childEntity);
          }
        }
      }
      else {
        uint32_t numChildren = 0;
        ofs.write(reinterpret_cast<const char*>(&numChildren), sizeof(numChildren));
      }

      return true;
    }

    void SerializeTransformComponent(std::ofstream& ofs, const TransformComponent& transform)
    {
      // Sérialisation de la position
      ofs.write(reinterpret_cast<const char*>(&transform.GetPosition()),
                sizeof(transform.GetPosition()));
      // Sérialisation de la rotation
      ofs.write(reinterpret_cast<const char*>(&transform.GetRotation()),
                sizeof(transform.GetRotation()));
      // Sérialisation de l'échelle
      ofs.write(reinterpret_cast<const char*>(&transform.GetScale()), sizeof(transform.GetScale()));
    }

    void SerializeMeshComponent(std::ofstream& ofs, const MeshComponent& meshComponent)
    {
      if (auto mesh = meshComponent.GetMesh()) {
        ofs.put(1); // Possède un maillage
        SerializeMesh(ofs, *mesh);
      }
      else {
        ofs.put(0); // Ne possède pas de maillage
      }
    }

    static void SerializePBRRenderer(std::ofstream& ofs, const PBRRenderer& renderer)
    {
      XMFLOAT4 baseColor = renderer.GetBaseColor();
      float    metallic = renderer.GetMetallic();
      float    roughness = renderer.GetRoughness();
      float    ao = renderer.GetAO();

      ofs.write(reinterpret_cast<const char*>(&baseColor), sizeof(baseColor));
      ofs.write(reinterpret_cast<const char*>(&metallic), sizeof(metallic));
      ofs.write(reinterpret_cast<const char*>(&roughness), sizeof(roughness));
      ofs.write(reinterpret_cast<const char*>(&ao), sizeof(ao));

      SerializeTextureReference(ofs, renderer.GetAlbedoTexture());
      SerializeTextureReference(ofs, renderer.GetNormalMap());
      SerializeTextureReference(ofs, renderer.GetAOMap());
    }


    static void SerializeTextureReference(std::ofstream& ofs, const Texture* texture)
    {
      if (texture) {
        ofs.put(1); // Texture présente
        std::wstring textureFilename = texture->GetFilename();
        uint32_t     length = static_cast<uint32_t>(textureFilename.size());
        ofs.write(reinterpret_cast<const char*>(&length), sizeof(length));
        ofs.write(reinterpret_cast<const char*>(textureFilename.data()), length * sizeof(wchar_t));
      }
      else {
        ofs.put(0); // Pas de texture
      }
    }

    void SerializeMesh(std::ofstream& ofs, const Mesh& mesh)
    {
      // Sérialisation des sommets
      const auto& vertices = mesh.GetVertices();
      uint32_t    numVertices = static_cast<uint32_t>(vertices.size());
      ofs.write(reinterpret_cast<const char*>(&numVertices), sizeof(numVertices));
      ofs.write(reinterpret_cast<const char*>(vertices.data()), numVertices * sizeof(Vertex));

      // Sérialisation des indices
      const auto& indices = mesh.GetIndices();
      uint32_t    numIndices = static_cast<uint32_t>(indices.size());
      ofs.write(reinterpret_cast<const char*>(&numIndices), sizeof(numIndices));
      ofs.write(reinterpret_cast<const char*>(indices.data()), numIndices * sizeof(uint32_t));
    }

    std::shared_ptr<Entity> DeserializeEntity(std::ifstream& ifs)
    {
      auto entity = World::GetInstance().CreateEntity();

      // Désérialisation de TransformComponent
      char hasTransform;
      ifs.get(hasTransform);
      if (hasTransform) {
        DeserializeTransformComponent(ifs, entity);
      }

      // Désérialisation de MeshComponent
      char hasMeshComponent;
      ifs.get(hasMeshComponent);
      if (hasMeshComponent) {
        DeserializeMeshComponent(ifs, entity);
      }

      // Désérialisation de PBRRenderer
      char hasPBRRenderer;
      ifs.get(hasPBRRenderer);
      if (hasPBRRenderer) {
        DeserializePBRRenderer(ifs, entity);
      }

      // Désérialisation de ColliderComponent
      char hasCollider;
      ifs.get(hasCollider);
      if (hasCollider) {
        ColliderComponent::Type colliderType;
        ifs.read(reinterpret_cast<char*>(&colliderType), sizeof(colliderType));
        auto& collider = entity->AddComponent<ColliderComponent>();
        collider.Deserialize(ifs);
      }

      // Désérialisation de RigidBodyComponent
      char hasRigidBody;
      ifs.get(hasRigidBody);
      if (hasRigidBody) {
        RigidBodyComponent::Type type;
        ifs.read(reinterpret_cast<char*>(&type), sizeof(type));
        auto& rigidBody = entity->AddComponent<RigidBodyComponent>(type);
        rigidBody.Initialize();
      }


      // Désérialisation des enfants
      uint32_t numChildren;
      ifs.read(reinterpret_cast<char*>(&numChildren), sizeof(numChildren));

      auto transform = entity->GetComponent<TransformComponent>();
      for (uint32_t i = 0; i < numChildren; ++i) {
        auto childEntity = DeserializeEntity(ifs);
        if (transform && childEntity) {
          transform->AddChild(childEntity->GetId());
        }
      }

      return entity;
    }

    static void DeserializeTransformComponent(std::ifstream&                 ifs,
                                              const std::shared_ptr<Entity>& entity)
    {
      XMFLOAT3 position;
      XMFLOAT4 rotation;
      XMFLOAT3 scale;

      ifs.read(reinterpret_cast<char*>(&position), sizeof(position));
      ifs.read(reinterpret_cast<char*>(&rotation), sizeof(rotation));
      ifs.read(reinterpret_cast<char*>(&scale), sizeof(scale));

      entity->AddComponent<TransformComponent>(position, rotation, scale);
    }

    void DeserializeMeshComponent(std::ifstream& ifs, const std::shared_ptr<Entity>& entity) const
    {
      auto& meshComponent = entity->AddComponent<MeshComponent>();
      char  hasMesh;
      ifs.get(hasMesh);
      if (hasMesh) {
        auto mesh = DeserializeMesh(ifs);
        meshComponent.SetMesh(mesh);
      }
    }

    void DeserializePBRRenderer(std::ifstream& ifs, std::shared_ptr<Entity> entity)
    {
      auto& renderer = entity->AddComponent<PBRRenderer>(m_pDispositif->GetD3DDevice());

      XMFLOAT4 baseColor;
      float    metallic;
      float    roughness;
      float    ao;

      ifs.read(reinterpret_cast<char*>(&baseColor), sizeof(baseColor));
      ifs.read(reinterpret_cast<char*>(&metallic), sizeof(metallic));
      ifs.read(reinterpret_cast<char*>(&roughness), sizeof(roughness));
      ifs.read(reinterpret_cast<char*>(&ao), sizeof(ao));

      renderer.SetBaseColor(baseColor);
      renderer.SetMetallic(metallic);
      renderer.SetRoughness(roughness);
      renderer.SetAmbientOcclusion(ao);

      if (auto* albedo = DeserializeTextureReference(ifs)) renderer.SetAlbedoTexture(albedo);
      if (auto* normal = DeserializeTextureReference(ifs)) renderer.SetNormalMap(normal);
      if (auto* occlusion = DeserializeTextureReference(ifs)) renderer.SetAOMap(occlusion);
      // TODO MetallicRoughnessMap
    }

    Texture* DeserializeTextureReference(std::ifstream& ifs) const
    {
      char hasTexture;
      ifs.get(hasTexture);
      if (hasTexture) {
        uint32_t length;
        ifs.read(reinterpret_cast<char*>(&length), sizeof(length));
        std::wstring textureFilename(length, L'\0');
        ifs.read(reinterpret_cast<char*>(textureFilename.data()), length * sizeof(wchar_t));

        return m_textureManager->GetNewTexture(textureFilename.c_str(), m_pDispositif);
      }
      return nullptr;
    }
    std::shared_ptr<Mesh> DeserializeMesh(std::ifstream& ifs) const
    {
      // Désérialisation des sommets
      uint32_t numVertices;
      ifs.read(reinterpret_cast<char*>(&numVertices), sizeof(numVertices));
      std::vector<Vertex> vertices(numVertices);
      ifs.read(reinterpret_cast<char*>(vertices.data()), numVertices * sizeof(Vertex));

      // Désérialisation des indices
      uint32_t numIndices;
      ifs.read(reinterpret_cast<char*>(&numIndices), sizeof(numIndices));
      std::vector<uint32_t> indices(numIndices);
      ifs.read(reinterpret_cast<char*>(indices.data()), numIndices * sizeof(uint32_t));

      // Création du maillage
      auto mesh = std::make_shared<Mesh>(m_pDispositif->GetD3DDevice(), vertices, indices);

      return mesh;
    }
  };
}
