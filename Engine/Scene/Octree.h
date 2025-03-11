#pragma once
#include <memory>
#include <vector>
#include <fstream>
#include <string>
#include "Engine/Math/AABB.h"
#include "Engine/Math/Frustum.h"

namespace FrostFireEngine
{
  class BaseRendererComponent;

  class OctreeNode {
  public:
    OctreeNode(const AABB& region, OctreeNode* parent = nullptr);

    bool IsLeaf() const;
    AABB GetLooseBounds(float looseness) const;

    void Insert(BaseRendererComponent* renderer,
                const AABB&            box,
                int                    maxDepth,
                int                    maxEntities,
                float                  looseness);
    bool Remove(BaseRendererComponent* renderer);

    bool UpdateRenderer(BaseRendererComponent* renderer,
                        const AABB&            newBox,
                        int                    maxDepth,
                        int                    maxEntities,
                        float                  looseness);

    void QueryFrustum(const Frustum& f, std::vector<BaseRendererComponent*>& out) const;
    void Clear();
    bool ContainsRenderer(const BaseRendererComponent* renderer) const;

    const std::vector<std::pair<BaseRendererComponent*, AABB>>& GetRenderers() const
    {
      return m_renderers;
    }
    const std::unique_ptr<OctreeNode>* GetChildren() const
    {
      return m_children;
    }
    const AABB& GetBounds() const
    {
      return m_bounds;
    }

  private:
    int  GetChildIndex(const AABB& box, float looseness) const;
    void Split();

    AABB                                                 m_bounds;
    OctreeNode*                                          m_parent = nullptr;
    std::unique_ptr<OctreeNode>                          m_children[8];
    std::vector<std::pair<BaseRendererComponent*, AABB>> m_renderers;
  };

  class Octree {
  public:
    Octree(const AABB& worldBounds,
           int         maxDepth = 8,
           int         maxEntitiesPerNode = 8,
           float       looseness = 1.0f);

    void InsertRenderer(BaseRendererComponent* renderer, const AABB& box);
    void RemoveRenderer(BaseRendererComponent* renderer) const;
    void UpdateRenderer(BaseRendererComponent* renderer, const AABB& box);
    void QueryFrustum(const Frustum& f, std::vector<BaseRendererComponent*>& outVisible) const;
    void Clear();

    AABB GetWorldBounds() const;

    void PrintToFile(const std::string& filename) const;

    const OctreeNode* GetRoot() const { return m_root.get(); }

  private:
    static void PrintNodeToFile(std::ofstream& file, const OctreeNode* node, int depth);

    AABB  m_worldBounds;
    int   m_maxDepth;
    int   m_maxEntities;
    float m_looseness;

    std::unique_ptr<OctreeNode> m_root;
  };
}
