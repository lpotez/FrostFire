#pragma once
#include "Engine/ECS/core/Component.h"
#include "Engine/Mesh.h"
#include <memory>
#include <vector>
#include <DirectXMath.h>

namespace FrostFireEngine
{
  struct LODLevel {
    float                 distance;
    std::shared_ptr<Mesh> mesh;

    LODLevel(float dist, const std::shared_ptr<Mesh>& m)
      : distance(dist), mesh(m)
    {
    }
  };

  class MeshComponent : public Component {
  public:
    MeshComponent();
    ~MeshComponent() override;

    void                  SetMesh(const std::shared_ptr<Mesh>& mesh);
    std::shared_ptr<Mesh> GetMesh() const;

    void AddLODLevel(float distance, const std::shared_ptr<Mesh>& mesh);
    void UpdateLOD(const DirectX::XMFLOAT3& cameraPosition);
    void ClearLODLevels();

    bool HasLODs() const
    {
      return !m_lodLevels.empty();
    }
    size_t GetCurrentLODIndex() const
    {
      return m_currentLODIndex;
    }

  private:
    std::shared_ptr<Mesh> m_baseMesh;
    std::vector<LODLevel> m_lodLevels;
    size_t                m_currentLODIndex;
  };
}
