#pragma once
#include "Engine/ECS/core/Component.h"
#include <DirectXMath.h>
#include <memory>
#include <iostream>
#include <vector>
#include <geometry/PxConvexMesh.h>
#include <geometry/PxTriangleMesh.h>

namespace physx
{
  class PxShape;
  class PxGeometry;
  class PxTriangleMesh;
  class PxConvexMesh;
}

namespace FrostFireEngine
{
  class PhysicsSystem;
  class Mesh;

  class ColliderComponent : public Component
  {
  public:
    enum class Type
    {
      Box,
      Sphere,
      Capsule,
      ConvexMesh,
      TriangleMesh
    };

    enum class MeshType
    {
      Convex,
      Triangle
    };

    explicit ColliderComponent(Type type = Type::Box);
    ~ColliderComponent() override;

    ColliderComponent(const ColliderComponent&) = delete;
    ColliderComponent& operator=(const ColliderComponent&) = delete;

    ColliderComponent(ColliderComponent&&) noexcept;
    ColliderComponent& operator=(ColliderComponent&&) noexcept;

    void Initialize(const DirectX::XMFLOAT3& size);
    void UpdateScale(const DirectX::XMFLOAT3& scale) const;

    const physx::PxGeometry& GetGeometry() const;
    void SetShape(physx::PxShape* newShape);
    physx::PxShape* GetShape() const { return shape; }
    Type GetType() const { return colliderType; }
    void SetMeshType(MeshType type) { meshType = type; }
    MeshType GetMeshType() const { return meshType; }

    void Serialize(std::ostream& out) const;
    void Deserialize(std::istream& in);
    static float CalculateMeshSize(const std::vector<physx::PxVec3>& vertices);

  private:
    struct GeometryHolder;
    std::unique_ptr<GeometryHolder> geometry;
    Type colliderType;
    MeshType meshType{ MeshType::Convex };
    physx::PxShape* shape{ nullptr };
    physx::PxConvexMesh* convexMesh{ nullptr };
    physx::PxTriangleMesh* triangleMesh{ nullptr };
  };
}
