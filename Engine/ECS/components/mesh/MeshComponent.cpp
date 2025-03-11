#include "MeshComponent.h"
#include "Engine/ECS/core/World.h"
#include "Engine/ECS/components/transform/TransformComponent.h"

namespace FrostFireEngine
{
  MeshComponent::MeshComponent() : m_currentLODIndex(0) {}
  MeshComponent::~MeshComponent() = default;

  void MeshComponent::SetMesh(const std::shared_ptr<Mesh>& mesh)
  {
    m_baseMesh = mesh;
  }

  std::shared_ptr<Mesh> MeshComponent::GetMesh() const
  {
    if (HasLODs()) {
      return m_lodLevels[m_currentLODIndex].mesh;
    }
    return m_baseMesh;
  }

  void MeshComponent::AddLODLevel(float distance, const std::shared_ptr<Mesh>& mesh)
  {
    m_lodLevels.emplace_back(distance, mesh);
    std::ranges::sort(m_lodLevels,
                      [](const LODLevel& a, const LODLevel& b) {
                        return a.distance > b.distance;
                      });
  }

  void MeshComponent::UpdateLOD(const XMFLOAT3& cameraPosition)
  {
    if (!HasLODs()) return;

    auto entity = World::GetInstance().GetEntity(GetOwner());
    auto transform = entity->GetComponent<TransformComponent>();
    if (!transform) return;

    XMVECTOR objectPos = XMLoadFloat3(&transform->GetPosition());
    XMVECTOR camPos = XMLoadFloat3(&cameraPosition);

    float distanceSq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(objectPos, camPos)));

    for (size_t i = 0; i < m_lodLevels.size(); ++i) {
      if (distanceSq > (m_lodLevels[i].distance * m_lodLevels[i].distance)) {
        m_currentLODIndex = i;
        return;
      }
    }

    m_currentLODIndex = m_lodLevels.size() - 1;
  }

  void MeshComponent::ClearLODLevels()
  {
    m_lodLevels.clear();
    m_currentLODIndex = 0;
  }
}
