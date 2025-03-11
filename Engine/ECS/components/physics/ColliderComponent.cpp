#include "ColliderComponent.h"

#include <geometry/PxBoxGeometry.h>
#include <geometry/PxCapsuleGeometry.h>
#include <geometry/PxConvexMeshGeometry.h>
#include <geometry/PxSphereGeometry.h>
#include <geometry/PxTriangleMeshGeometry.h>

#include "RigidBodyComponent.h"
#include "Engine/ECS/components/mesh/MeshComponent.h"
#include "Engine/ECS/components/transform/TransformComponent.h"
#include "Engine/ECS/systems/PhysicsSystem.h"


namespace FrostFireEngine
{
  using namespace DirectX;

  #undef min
  #undef max

  struct ColliderComponent::GeometryHolder {
    union {
      physx::PxBoxGeometry          box;
      physx::PxSphereGeometry       sphere;
      physx::PxCapsuleGeometry      capsule;
      physx::PxTriangleMeshGeometry triangleMesh;
      physx::PxConvexMeshGeometry   convexMesh;
    };

    XMFLOAT3 originalSize;

    explicit GeometryHolder()
      : box(physx::PxBoxGeometry(0.5f, 0.5f, 0.5f))
        , originalSize(1.0f, 1.0f, 1.0f)
    {
    }
    ~GeometryHolder() = default;
  };

  ColliderComponent::ColliderComponent(Type type)
    : geometry(std::make_unique<GeometryHolder>())
      , colliderType(type)
  {
  }

  ColliderComponent::~ColliderComponent()
  {
    if (convexMesh) {
      convexMesh->release();
      convexMesh = nullptr;
    }
    if (triangleMesh) {
      triangleMesh->release();
      triangleMesh = nullptr;
    }
  }

  ColliderComponent::ColliderComponent(ColliderComponent&& other) noexcept
    : geometry(std::move(other.geometry))
      , colliderType(other.colliderType)
      , meshType(other.meshType)
      , shape(other.shape)
      , convexMesh(other.convexMesh)
      , triangleMesh(other.triangleMesh)
  {
    other.shape = nullptr;
    other.convexMesh = nullptr;
    other.triangleMesh = nullptr;
  }

  ColliderComponent& ColliderComponent::operator=(ColliderComponent&& other) noexcept
  {
    if (this != &other) {
      colliderType = other.colliderType;
      meshType = other.meshType;
      geometry = std::move(other.geometry);
      shape = other.shape;
      convexMesh = other.convexMesh;
      triangleMesh = other.triangleMesh;

      other.shape = nullptr;
      other.convexMesh = nullptr;
      other.triangleMesh = nullptr;
    }
    return *this;
  }
  void ColliderComponent::Initialize(const XMFLOAT3& size)
  {
    geometry->originalSize = size;

    const auto& entityManager = World::GetInstance();
    const auto& entityOwner = entityManager.GetEntity(owner);
    XMFLOAT3    scale = {1.0f, 1.0f, 1.0f};

    if (const auto* transform = entityOwner->GetComponent<TransformComponent>()) {
      scale = transform->GetScale();
    }

    physx::PxPhysics* physics = PhysicsSystem::Get().GetPhysics();

    switch (colliderType) {
      case Type::Box:
        new(&geometry->box) physx::PxBoxGeometry(
          size.x * scale.x * 0.5f,
          size.y * scale.y * 0.5f,
          size.z * scale.z * 0.5f
        );
        break;

      case Type::Sphere:
        {
          const float maxScale = std::max({scale.x, scale.y, scale.z});
          new(&geometry->sphere) physx::PxSphereGeometry(
            size.x * maxScale * 0.5f
          );
        }
        break;

      case Type::Capsule:
        {
          const float radiusScale = std::max(scale.x, scale.z);
          new(&geometry->capsule) physx::PxCapsuleGeometry(
            size.x * radiusScale * 0.5f,
            size.y * scale.y * 0.5f
          );
        }
        break;

      case Type::ConvexMesh:
      case Type::TriangleMesh:
        {
          if (entityOwner) {
            if (auto* meshComponent = entityOwner->GetComponent<MeshComponent>()) {
              if (const auto mesh = meshComponent->GetMesh()) {
                const std::vector<Vertex>&   vertices = mesh->GetVertices();
                const std::vector<uint32_t>& indices = mesh->GetIndices();
                std::vector<physx::PxVec3>   pxVertices(vertices.size());

                for (size_t i = 0; i < vertices.size(); ++i) {
                  pxVertices[i] = physx::PxVec3(
                    vertices[i].GetPosition().x,
                    vertices[i].GetPosition().y,
                    vertices[i].GetPosition().z
                  );
                }

                if (colliderType == Type::ConvexMesh) {
                  physx::PxConvexMeshDesc convexDesc;
                  convexDesc.points.count = static_cast<uint32_t>(pxVertices.size());
                  convexDesc.points.stride = sizeof(physx::PxVec3);
                  convexDesc.points.data = pxVertices.data();
                  convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX |
                  physx::PxConvexFlag::eDISABLE_MESH_VALIDATION |
                  physx::PxConvexFlag::eQUANTIZE_INPUT |
                  physx::PxConvexFlag::eSHIFT_VERTICES;

                  physx::PxCookingParams cookingParams(physics->getTolerancesScale());
                  cookingParams.planeTolerance = 0.0001f;
                  cookingParams.meshPreprocessParams =
                  physx::PxMeshPreprocessingFlag::eWELD_VERTICES;
                  cookingParams.gaussMapLimit = 32;
                  cookingParams.areaTestEpsilon = 0.001f;
                  cookingParams.convexMeshCookingType = physx::PxConvexMeshCookingType::eQUICKHULL;

                  physx::PxDefaultMemoryOutputStream writeBuffer;
                  if (!PxCookConvexMesh(cookingParams, convexDesc, writeBuffer)) {
                    throw std::runtime_error("Failed to cook convex mesh.");
                  }

                  physx::PxDefaultMemoryInputData readBuffer(
                    writeBuffer.getData(), writeBuffer.getSize());
                  convexMesh = physics->createConvexMesh(readBuffer);

                  new(&geometry->convexMesh) physx::PxConvexMeshGeometry(
                    convexMesh,
                    physx::PxMeshScale(physx::PxVec3(scale.x, scale.y, scale.z))
                  );
                }
                else // Type::TriangleMesh
                {
                  physx::PxTriangleMeshDesc triangleDesc;
                  triangleDesc.points.count = static_cast<uint32_t>(pxVertices.size());
                  triangleDesc.points.stride = sizeof(physx::PxVec3);
                  triangleDesc.points.data = pxVertices.data();
                  triangleDesc.triangles.count = static_cast<uint32_t>(indices.size() / 3);
                  triangleDesc.triangles.stride = 3 * sizeof(uint32_t);
                  triangleDesc.triangles.data = indices.data();

                  physx::PxCookingParams cookingParams(physics->getTolerancesScale());
                  cookingParams.meshPreprocessParams =
                  physx::PxMeshPreprocessingFlag::eWELD_VERTICES;
                  cookingParams.suppressTriangleMeshRemapTable = true;

                  physx::PxDefaultMemoryOutputStream writeBuffer;
                  if (!PxCookTriangleMesh(cookingParams, triangleDesc, writeBuffer)) {
                    throw std::runtime_error("Failed to cook triangle mesh.");
                  }

                  physx::PxDefaultMemoryInputData readBuffer(
                    writeBuffer.getData(), writeBuffer.getSize());
                  triangleMesh = physics->createTriangleMesh(readBuffer);

                  new(&geometry->triangleMesh) physx::PxTriangleMeshGeometry(
                    triangleMesh,
                    physx::PxMeshScale(physx::PxVec3(scale.x, scale.y, scale.z))
                  );
                }
              }
            }
          }
        }
        break;
    }
  }
  void ColliderComponent::UpdateScale(const XMFLOAT3& scale) const
  {
    switch (colliderType) {
      case Type::Box:
        geometry->box.halfExtents = physx::PxVec3(
          geometry->originalSize.x * scale.x * 0.5f,
          geometry->originalSize.y * scale.y * 0.5f,
          geometry->originalSize.z * scale.z * 0.5f
        );
        break;

      case Type::Sphere:
        {
          float maxScale = std::max({scale.x, scale.y, scale.z});
          geometry->sphere.radius = geometry->originalSize.x * maxScale * 0.5f;
        }
        break;

      case Type::Capsule:
        geometry->capsule.radius = geometry->originalSize.x * scale.x * 0.5f;
        geometry->capsule.halfHeight = geometry->originalSize.y * scale.y * 0.5f;
        break;

      case Type::ConvexMesh:
        geometry->convexMesh.scale = physx::PxMeshScale(
          physx::PxVec3(scale.x, scale.y, scale.z)
        );
        break;

      case Type::TriangleMesh:
        geometry->triangleMesh.scale = physx::PxMeshScale(
          physx::PxVec3(scale.x, scale.y, scale.z)
        );
        break;
    }

    if (shape) {
      shape->setGeometry(GetGeometry());

      const auto entityOwner = World::GetInstance().GetEntity(owner);
      if (entityOwner) {
        if (auto* rigidBody = entityOwner->GetComponent<RigidBodyComponent>()) {
          rigidBody->UpdateMassProperties();
        }
      }
    }
  }

  const physx::PxGeometry& ColliderComponent::GetGeometry() const
  {
    switch (colliderType) {
      case Type::Box:
        return geometry->box;
      case Type::Sphere:
        return geometry->sphere;
      case Type::Capsule:
        return geometry->capsule;
      case Type::ConvexMesh:
        return geometry->convexMesh;
      case Type::TriangleMesh:
        return geometry->triangleMesh;
      default:
        return geometry->box;
    }
  }
  void ColliderComponent::SetShape(physx::PxShape* newShape)
  {
    shape = newShape;
  }

  void ColliderComponent::Serialize(std::ostream& out) const
  {
    out.write(reinterpret_cast<const char*>(&colliderType), sizeof(colliderType));
    out.write(reinterpret_cast<const char*>(&meshType), sizeof(meshType));

    switch (colliderType) {
      case Type::Box:
        {
          const physx::PxVec3& halfExtents = geometry->box.halfExtents;
          out.write(reinterpret_cast<const char*>(&halfExtents), sizeof(halfExtents));
        }
        break;

      case Type::Sphere:
        {
          float radius = geometry->sphere.radius;
          out.write(reinterpret_cast<const char*>(&radius), sizeof(radius));
        }
        break;

      case Type::Capsule:
        {
          float radius = geometry->capsule.radius;
          float halfHeight = geometry->capsule.halfHeight;
          out.write(reinterpret_cast<const char*>(&radius), sizeof(radius));
          out.write(reinterpret_cast<const char*>(&halfHeight), sizeof(halfHeight));
        }
        break;

      case Type::ConvexMesh:
      case Type::TriangleMesh:
        {
          const auto entityOwner = World::GetInstance().GetEntity(owner);
          if (entityOwner) {
            if (const auto* meshComponent = entityOwner->GetComponent<MeshComponent>()) {
              if (const auto mesh = meshComponent->GetMesh()) {
                const std::vector<Vertex>&   vertices = mesh->GetVertices();
                const std::vector<uint32_t>& indices = mesh->GetIndices();

                uint32_t vertexCount = static_cast<uint32_t>(vertices.size());
                out.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
                out.write(reinterpret_cast<const char*>(vertices.data()),
                          vertexCount * sizeof(Vertex));

                uint32_t indexCount = static_cast<uint32_t>(indices.size());
                out.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));
                out.write(reinterpret_cast<const char*>(indices.data()),
                          indexCount * sizeof(uint32_t));
              }
            }
          }
        }
        break;
    }
  }
  void ColliderComponent::Deserialize(std::istream& in)
  {
    in.read(reinterpret_cast<char*>(&colliderType), sizeof(colliderType));
    in.read(reinterpret_cast<char*>(&meshType), sizeof(meshType));

    physx::PxPhysics* physics = PhysicsSystem::Get().GetPhysics();

    switch (colliderType) {
      case Type::Box:
        {
          physx::PxVec3 halfExtents;
          in.read(reinterpret_cast<char*>(&halfExtents), sizeof(halfExtents));
          new(&geometry->box) physx::PxBoxGeometry(halfExtents);
        }
        break;

      case Type::Sphere:
        {
          float radius;
          in.read(reinterpret_cast<char*>(&radius), sizeof(radius));
          new(&geometry->sphere) physx::PxSphereGeometry(radius);
        }
        break;

      case Type::Capsule:
        {
          float radius, halfHeight;
          in.read(reinterpret_cast<char*>(&radius), sizeof(radius));
          in.read(reinterpret_cast<char*>(&halfHeight), sizeof(halfHeight));
          new(&geometry->capsule) physx::PxCapsuleGeometry(radius, halfHeight);
        }
        break;

      case Type::ConvexMesh:
      case Type::TriangleMesh:
        {
          uint32_t vertexCount = 0;
          in.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
          std::vector<Vertex> vertices(vertexCount);
          in.read(reinterpret_cast<char*>(vertices.data()), vertexCount * sizeof(Vertex));

          uint32_t indexCount = 0;
          in.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
          std::vector<uint32_t> indices(indexCount);
          in.read(reinterpret_cast<char*>(indices.data()), indexCount * sizeof(uint32_t));

          std::vector<physx::PxVec3> pxVertices(vertexCount);
          for (size_t i = 0; i < vertices.size(); ++i) {
            pxVertices[i] = physx::PxVec3(
              vertices[i].GetPosition().x,
              vertices[i].GetPosition().y,
              vertices[i].GetPosition().z
            );
          }

          if (colliderType == Type::ConvexMesh) {
            physx::PxConvexMeshDesc convexDesc;
            convexDesc.points.count = static_cast<uint32_t>(pxVertices.size());
            convexDesc.points.stride = sizeof(physx::PxVec3);
            convexDesc.points.data = pxVertices.data();
            convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX |
            physx::PxConvexFlag::eDISABLE_MESH_VALIDATION |
            physx::PxConvexFlag::eQUANTIZE_INPUT |
            physx::PxConvexFlag::eSHIFT_VERTICES;

            physx::PxCookingParams cookingParams(physics->getTolerancesScale());
            cookingParams.planeTolerance = 0.0001f;
            cookingParams.meshPreprocessParams = physx::PxMeshPreprocessingFlag::eWELD_VERTICES;
            cookingParams.gaussMapLimit = 32;
            cookingParams.areaTestEpsilon = 0.001f;
            cookingParams.convexMeshCookingType = physx::PxConvexMeshCookingType::eQUICKHULL;

            physx::PxDefaultMemoryOutputStream writeBuffer;
            bool status = PxCookConvexMesh(cookingParams, convexDesc, writeBuffer);
            if (!status) {
              throw std::runtime_error("Failed to cook convex mesh during deserialization.");
            }

            physx::PxDefaultMemoryInputData
            readBuffer(writeBuffer.getData(), writeBuffer.getSize());
            convexMesh = physics->createConvexMesh(readBuffer);
            new(&geometry->convexMesh) physx::PxConvexMeshGeometry(convexMesh);
          }
          else // Type::TriangleMesh
          {
            physx::PxTriangleMeshDesc triangleDesc;
            triangleDesc.points.count = static_cast<uint32_t>(pxVertices.size());
            triangleDesc.points.stride = sizeof(physx::PxVec3);
            triangleDesc.points.data = pxVertices.data();
            triangleDesc.triangles.count = static_cast<uint32_t>(indices.size() / 3);
            triangleDesc.triangles.stride = 3 * sizeof(uint32_t);
            triangleDesc.triangles.data = indices.data();

            float meshSize = CalculateMeshSize(pxVertices);
            float weldTolerance = meshSize * 0.001f; // 0.1% of mesh size

            physx::PxCookingParams cookingParams(physics->getTolerancesScale());
            cookingParams.meshPreprocessParams = physx::PxMeshPreprocessingFlag::eWELD_VERTICES;
            cookingParams.suppressTriangleMeshRemapTable = true;
            cookingParams.meshWeldTolerance = weldTolerance;

            physx::PxDefaultMemoryOutputStream writeBuffer;
            bool status = PxCookTriangleMesh(cookingParams, triangleDesc, writeBuffer);
            if (!status) {
              throw std::runtime_error("Failed to cook triangle mesh during deserialization.");
            }

            physx::PxDefaultMemoryInputData
            readBuffer(writeBuffer.getData(), writeBuffer.getSize());
            triangleMesh = physics->createTriangleMesh(readBuffer);
            new(&geometry->triangleMesh) physx::PxTriangleMeshGeometry(triangleMesh);
          }
        }
        break;
    }
  };

  float ColliderComponent::CalculateMeshSize(const std::vector<physx::PxVec3>& vertices)
  {
    if (vertices.empty()) return 1.0f;

    physx::PxVec3 minBounds(FLT_MAX);
    physx::PxVec3 maxBounds(-FLT_MAX);

    for (const auto& vertex : vertices) {
      minBounds.x = std::min(minBounds.x, vertex.x);
      minBounds.y = std::min(minBounds.y, vertex.y);
      minBounds.z = std::min(minBounds.z, vertex.z);

      maxBounds.x = std::max(maxBounds.x, vertex.x);
      maxBounds.y = std::max(maxBounds.y, vertex.y);
      maxBounds.z = std::max(maxBounds.z, vertex.z);
    }

    physx::PxVec3 size = maxBounds - minBounds;
    return std::max({size.x, size.y, size.z});
  }
}
