#pragma once
#include <DirectXMath.h>
#include <vector>
#include <limits>
#include "Engine/ECS/core/Entity.h"
#include "Engine/ECS/components/rendering/BaseRendererComponent.h"
#include "Engine/ECS/components/CameraComponent.h"
#include "Engine/ECS/components/transform/TransformComponent.h"
#include "Engine/ECS/core/World.h"
#include "Engine/Math/AABB.h"
#include "Engine/DispositifD3D11.h"
#include "Engine/ECS/components/mesh/MeshComponent.h"

namespace FrostFireEngine
{
  #undef min
    #undef max
  class Raycaster {
  public:
    struct Ray {
      XMFLOAT3 origin;
      XMFLOAT3 direction;
    };

    struct RaycastHit {
      BaseRendererComponent* renderer = nullptr;
      float                  distance = std::numeric_limits<float>::max();
    };

    Raycaster() = default;
    ~Raycaster() = default;

    // Construit un rayon dans l'espace monde à partir des coordonnées écran (x, y) de la souris.
    // x, y sont en coordonnées viewport [0, viewportWidth] x [0, viewportHeight]
    // camera est la caméra active.
    // device est pour obtenir les dimensions du viewport.
    static Ray ScreenPointToRay(float                  x,
                                float                  y,
                                const CameraComponent* camera,
                                const DispositifD3D11* device)
    {
      using namespace DirectX;

      float viewportWidth = device->GetViewportWidth();
      float viewportHeight = device->GetViewportHeight();

      // Normalisation en coordonnées Normalized Device Coordinates [-1,1]
      float px = ((2.0f * x) / viewportWidth) - 1.0f;
      float py = 1.0f - ((2.0f * y) / viewportHeight); // inversion de y

      XMMATRIX invViewProj = XMMatrixInverse(nullptr, camera->GetViewProjMatrix());
      XMVECTOR rayStartNDC = XMVectorSet(px, py, 0.0f, 1.0f);
      XMVECTOR rayEndNDC = XMVectorSet(px, py, 1.0f, 1.0f);

      XMVECTOR rayStartWorld = XMVector3TransformCoord(rayStartNDC, invViewProj);
      XMVECTOR rayEndWorld = XMVector3TransformCoord(rayEndNDC, invViewProj);

      XMFLOAT3 start, end;
      XMStoreFloat3(&start, rayStartWorld);
      XMStoreFloat3(&end, rayEndWorld);

      XMFLOAT3 dir = {
        end.x - start.x,
        end.y - start.y,
        end.z - start.z
      };
      XMStoreFloat3(&dir, XMVector3Normalize(XMLoadFloat3(&dir)));

      Ray ray;
      ray.origin = start;
      ray.direction = dir;
      return ray;
    }

    // Teste l'intersection entre un rayon et une AABB. Retourne true et la distance si intersection.
    static bool RayIntersectAABB(const Ray& ray, const AABB& box, float& tminOut)
    {
      using namespace DirectX;

      // Méthode standard pour intersection rayon/boîte axis-aligned
      XMFLOAT3 invDir = {
        1.0f / ray.direction.x,
        1.0f / ray.direction.y,
        1.0f / ray.direction.z
      };

      float t1 = (box.min.x - ray.origin.x) * invDir.x;
      float t2 = (box.max.x - ray.origin.x) * invDir.x;

      float tmin = std::min(t1, t2);
      float tmax = std::max(t1, t2);

      t1 = (box.min.y - ray.origin.y) * invDir.y;
      t2 = (box.max.y - ray.origin.y) * invDir.y;

      tmin = std::max(tmin, std::min(t1, t2));
      tmax = std::min(tmax, std::max(t1, t2));

      t1 = (box.min.z - ray.origin.z) * invDir.z;
      t2 = (box.max.z - ray.origin.z) * invDir.z;

      tmin = std::max(tmin, std::min(t1, t2));
      tmax = std::min(tmax, std::max(t1, t2));

      if (tmax >= tmin && tmax > 0) {
        // Le rayon intersecte la boîte
        if (tmin > 0) {
          tminOut = tmin;
        }
        else {
          tminOut = tmax;
        }
        return true;
      }

      return false;
    }

    // Effectue un raycast dans la scène. On passe une liste de renderers sur lesquels tester.
    // Retourne le hit le plus proche s'il y en a un.
    static RaycastHit RaycastScene(const Ray&                                 ray,
                                   const std::vector<BaseRendererComponent*>& renderers)
    {
      RaycastHit bestHit;

      for (auto* r : renderers) {
        auto entity = World::GetInstance().GetEntity(r->GetOwner());
        if (!entity) continue;

        auto meshComp = entity->GetComponent<MeshComponent>();
        auto transform = entity->GetComponent<TransformComponent>();
        if (!meshComp || !transform) continue;
        auto mesh = meshComp->GetMesh();
        if (!mesh) continue;

        // Récupération de l'AABB du mesh en world space
        auto     localBounds = mesh->GetBounds().box;
        XMMATRIX worldMatrix = transform->GetWorldMatrix();

        // Transformer AABB local en world AABB
        XMFLOAT3 corners[8] = {
          {localBounds.min.x, localBounds.min.y, localBounds.min.z},
          {localBounds.min.x, localBounds.min.y, localBounds.max.z},
          {localBounds.min.x, localBounds.max.y, localBounds.min.z},
          {localBounds.min.x, localBounds.max.y, localBounds.max.z},
          {localBounds.max.x, localBounds.min.y, localBounds.min.z},
          {localBounds.max.x, localBounds.min.y, localBounds.max.z},
          {localBounds.max.x, localBounds.max.y, localBounds.min.z},
          {localBounds.max.x, localBounds.max.y, localBounds.max.z},
        };

        XMFLOAT3 worldMin = {FLT_MAX, FLT_MAX, FLT_MAX};
        XMFLOAT3 worldMax = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
        for (auto& c : corners) {
          XMVECTOR v = XMLoadFloat3(&c);
          v = XMVector3Transform(v, worldMatrix);
          XMFLOAT3 wc;
          XMStoreFloat3(&wc, v);
          worldMin.x = std::min(worldMin.x, wc.x);
          worldMin.y = std::min(worldMin.y, wc.y);
          worldMin.z = std::min(worldMin.z, wc.z);
          worldMax.x = std::max(worldMax.x, wc.x);
          worldMax.y = std::max(worldMax.y, wc.y);
          worldMax.z = std::max(worldMax.z, wc.z);
        }

        AABB  worldAABB{worldMin, worldMax};
        float t;
        if (RayIntersectAABB(ray, worldAABB, t)) {
          if (t < bestHit.distance) {
            bestHit.distance = t;
            bestHit.renderer = r;
          }
        }
      }

      return bestHit;
    }
  };
}
