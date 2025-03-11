#include "Octree.h"
#include "Engine/ECS/components/rendering/BaseRendererComponent.h"

#include <DirectXMath.h>
#include <algorithm>
#include <fstream>
#include <ranges>

namespace FrostFireEngine
{
  using namespace DirectX;

  OctreeNode::OctreeNode(const AABB& region, OctreeNode* parent)
    : m_bounds(region), m_parent(parent)
  {
  }

  bool OctreeNode::IsLeaf() const
  {
    for (auto& c : m_children) if (c) return false;
    return true;
  }

  AABB OctreeNode::GetLooseBounds(float looseness) const
  {
    XMFLOAT3 c = m_bounds.Center();
    XMFLOAT3 e = m_bounds.Extents();
    e.x *= looseness;
    e.y *= looseness;
    e.z *= looseness;
    AABB loose;
    loose.min = {c.x - e.x, c.y - e.y, c.z - e.z};
    loose.max = {c.x + e.x, c.y + e.y, c.z + e.z};
    return loose;
  }

  int OctreeNode::GetChildIndex(const AABB& box, float looseness) const
  {
    XMFLOAT3 c = m_bounds.Center();
    XMFLOAT3 bc = box.Center();
    bool     x = (bc.x > c.x);
    bool     y = (bc.y > c.y);
    bool     z = (bc.z > c.z);
    int      index = (x ? 1 : 0) + (y ? 2 : 0) + (z ? 4 : 0);

    AABB childBounds = m_bounds;
    if (x) childBounds.min.x = c.x;
    else childBounds.max.x = c.x;
    if (y) childBounds.min.y = c.y;
    else childBounds.max.y = c.y;
    if (z) childBounds.min.z = c.z;
    else childBounds.max.z = c.z;

    AABB     loose = childBounds;
    XMFLOAT3 ext = loose.Extents();
    XMFLOAT3 cent = loose.Center();
    ext.x *= looseness;
    ext.y *= looseness;
    ext.z *= looseness;
    loose.min = {cent.x - ext.x, cent.y - ext.y, cent.z - ext.z};
    loose.max = {cent.x + ext.x, cent.y + ext.y, cent.z + ext.z};

    return loose.Contains(box) ? index : -1;
  }

  void OctreeNode::Split()
  {
    if (!IsLeaf()) return;
    XMFLOAT3 c = m_bounds.Center();

    for (int i = 0; i < 8; i++) {
      AABB childBounds = m_bounds;
      if (i & 1) childBounds.min.x = c.x;
      else childBounds.max.x = c.x;
      if (i & 2) childBounds.min.y = c.y;
      else childBounds.max.y = c.y;
      if (i & 4) childBounds.min.z = c.z;
      else childBounds.max.z = c.z;
      m_children[i] = std::make_unique<OctreeNode>(childBounds, this);
    }
  }

  void OctreeNode::Insert(BaseRendererComponent* renderer,
                          const AABB&            box,
                          int                    maxDepth,
                          int                    maxEntities,
                          float                  looseness)
  {
    if (maxDepth > 0 && IsLeaf() && static_cast<int>(m_renderers.size()) >= maxEntities) {
      Split();
    }
    if (!IsLeaf()) {
      int childIndex = GetChildIndex(box, looseness);
      if (childIndex != -1) {
        m_children[childIndex]->Insert(renderer, box, maxDepth - 1, maxEntities, looseness);
        return;
      }
    }
    m_renderers.push_back({renderer, box});
  }

  bool OctreeNode::Remove(BaseRendererComponent* renderer)
  {
    for (auto it = m_renderers.begin(); it != m_renderers.end(); ++it) {
      if (it->first == renderer) {
        m_renderers.erase(it);
        return true;
      }
    }

    if (!IsLeaf()) {
      for (auto& c : m_children) {
        if (c && c->Remove(renderer)) return true;
      }
    }
    return false;
  }

  bool OctreeNode::UpdateRenderer(BaseRendererComponent* renderer,
                                  const AABB&            newBox,
                                  int                    maxDepth,
                                  int                    maxEntities,
                                  float                  looseness)
  {
    for (auto it = m_renderers.begin(); it != m_renderers.end(); ++it) {
      if (it->first == renderer) {
        AABB loose = GetLooseBounds(looseness);
        if (loose.Contains(newBox)) {
          it->second = newBox;
          return true;
        }
        m_renderers.erase(it);
        OctreeNode* current = this;
        while (current->m_parent) {
          current = current->m_parent;
          AABB parentLoose = current->GetLooseBounds(looseness);
          if (parentLoose.Contains(newBox)) {
            current->Insert(renderer, newBox, maxDepth, maxEntities, looseness);
            return true;
          }
        }
        current->Insert(renderer, newBox, maxDepth, maxEntities, looseness);
        return true;
      }
    }

    if (!IsLeaf()) {
      for (auto& c : m_children) {
        if (c && c->UpdateRenderer(renderer, newBox, maxDepth, maxEntities, looseness)) return true;
      }
    }
    return false;
  }

  void OctreeNode::QueryFrustum(const Frustum& f, std::vector<BaseRendererComponent*>& out) const
  {
    if (!f.CheckBox(m_bounds)) return;
    for (auto& r : m_renderers) {
      if (f.CheckBox(r.second)) out.push_back(r.first);
    }
    if (!IsLeaf()) {
      for (auto& c : m_children) {
        if (c) c->QueryFrustum(f, out);
      }
    }
  }

  void OctreeNode::Clear()
  {
    m_renderers.clear();
    m_renderers.shrink_to_fit();
    for (auto& c : m_children) {
      if (c) {
        c->Clear();
        c.reset();
      }
    }
  }

  Octree::Octree(const AABB& worldBounds,
                 int         maxDepth,
                 int         maxEntitiesPerNode,
                 float       looseness)
    : m_worldBounds(worldBounds),
      m_maxDepth(maxDepth),
      m_maxEntities(maxEntitiesPerNode),
      m_looseness(looseness)
  {
    m_root = std::make_unique<OctreeNode>(worldBounds);
  }

  void Octree::InsertRenderer(BaseRendererComponent* renderer, const AABB& box) 
  {
    if (!m_root) {
      m_root = std::make_unique<OctreeNode>(m_worldBounds);
    }
    m_root->Insert(renderer, box, m_maxDepth, m_maxEntities, m_looseness);
  }

  void Octree::RemoveRenderer(BaseRendererComponent* renderer) const
  {
    m_root->Remove(renderer);
  }

  void Octree::UpdateRenderer(BaseRendererComponent* renderer, const AABB& box)
  {
    if (!m_root->UpdateRenderer(renderer, box, m_maxDepth, m_maxEntities, m_looseness)) {
      InsertRenderer(renderer, box);
    }
  }

  void Octree::QueryFrustum(const Frustum& f, std::vector<BaseRendererComponent*>& outVisible) const
  {
    m_root->QueryFrustum(f, outVisible);
  }

  void Octree::Clear()
  {
    m_root->Clear();
    m_root.reset(); 
  }

  AABB Octree::GetWorldBounds() const
  {
    if (!m_root) {
      return m_worldBounds;
    }

    XMFLOAT3 globalMin(FLT_MAX, FLT_MAX, FLT_MAX);
    XMFLOAT3 globalMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    // Parcourir l'octree en profondeur (stack)
    std::vector<const OctreeNode *> stack;
    stack.push_back(m_root.get());

    while (!stack.empty()) {
      const OctreeNode *node = stack.back();
      stack.pop_back();

      // Pour chaque renderer dans ce node
      for (auto &r : node->GetRenderers()) {
        const AABB &rb = r.second;
        globalMin.x = std::min(globalMin.x, rb.min.x);
        globalMin.y = std::min(globalMin.y, rb.min.y);
        globalMin.z = std::min(globalMin.z, rb.min.z);

        globalMax.x = std::max(globalMax.x, rb.max.x);
        globalMax.y = std::max(globalMax.y, rb.max.y);
        globalMax.z = std::max(globalMax.z, rb.max.z);
      }

      // Ajouter les enfants à la stack
      const auto *children = node->GetChildren();
      for (int i = 0; i < 8; i++) {
        if (children[i]) {
          stack.push_back(children[i].get());
        }
      }
    }

    // Si aucun objet n'a été trouvé (octree vide en termes d'objets), on retourne m_worldBounds
    if (globalMin.x == FLT_MAX) {
      return m_worldBounds;
    }

    AABB computedBounds;
    computedBounds.min = globalMin;
    computedBounds.max = globalMax;
    return computedBounds;
  }


  void Octree::PrintToFile(const std::string& filename) const
  {
    std::ofstream file(filename);
    if (!file.is_open()) {
      return;
    }

    file << "Octree Structure:\n";
    file << "World Bounds: [("
    << m_worldBounds.min.x << ", " << m_worldBounds.min.y << ", " << m_worldBounds.min.z << "), ("
    << m_worldBounds.max.x << ", " << m_worldBounds.max.y << ", " << m_worldBounds.max.z << ")]\n";
    file << "Max Depth: " << m_maxDepth << "\n";
    file << "Max Entities per Node: " << m_maxEntities << "\n";
    file << "Looseness: " << m_looseness << "\n\n";

    PrintNodeToFile(file, m_root.get(), 0);
    file.close();
  }

  void Octree::PrintNodeToFile(std::ofstream& file, const OctreeNode* node, int depth)
  {
    if (!node) return;
    std::string indent(depth * 2, ' ');

    const AABB& b = node->GetBounds();
    file << indent << "Node at depth " << depth << ":\n";
    file << indent << "Bounds: [("
    << b.min.x << ", " << b.min.y << ", " << b.min.z << "), ("
    << b.max.x << ", " << b.max.y << ", " << b.max.z << ")]\n";

    const auto& renderers = node->GetRenderers();
    file << indent << "Renderers count: " << renderers.size() << "\n";

    for (size_t i = 0; i < renderers.size(); ++i) {
      const AABB&    rb = renderers[i].second;
      const EntityId entityId = renderers[i].first->GetOwner();
      file << indent << "  Renderer " << entityId << ": AABB [("
      << rb.min.x << ", " << rb.min.y << ", " << rb.min.z << "), ("
      << rb.max.x << ", " << rb.max.y << ", " << rb.max.z << ")]\n";
    }

    const auto* children = node->GetChildren();
    for (int i = 0; i < 8; ++i) {
      if (children[i]) {
        PrintNodeToFile(file, children[i].get(), depth + 1);
      }
    }
  }
  bool OctreeNode::ContainsRenderer(const BaseRendererComponent* renderer) const
  {
    for (const auto& key : m_renderers | std::views::keys) {
      if (key == renderer) {
        return true;
      }
    }
    return false;
  }
}
