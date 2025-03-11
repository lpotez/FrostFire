#pragma once
#include <memory>
#include <string>
#include <vector>

#include "FBXData.h"
#include "Mesh.h"

namespace FrostFireEngine
{
    struct MeshNode
    {
        std::string name;
        std::shared_ptr<Mesh> mesh;
        FBXMaterial material;
        FBXNodeTransform transform;
        std::vector<std::shared_ptr<MeshNode>> children;
        MeshNode* parent;
        bool isValid;

        MeshNode()
            : name("")
              , mesh(nullptr)
              , parent(nullptr)
              , isValid(false)
        {
        }

        void AddChild(const std::shared_ptr<MeshNode>& child)
        {
            if (child)
            {
                child->parent = this;
                children.push_back(child);
            }
        }

        bool HasMesh() const
        {
            return mesh != nullptr;
        }

        bool HasValidMaterial() const
        {
            return material.diffuseMap.length() > 0 ||
            (material.diffuse.x != 0.0f &&
                material.diffuse.y != 0.0f &&
                material.diffuse.z != 0.0f);
        }
    };
}
