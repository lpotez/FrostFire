#pragma once
#include <stdexcept>
#include <PxPhysicsAPI.h>
#include "Engine/Singleton.h"

namespace FrostFireEngine
{
  class PhysicsResources : public CSingleton<PhysicsResources> {
    friend class CSingleton<PhysicsResources>;

  public:
    physx::PxPhysics* GetPhysics() const
    {
      return physics;
    }
    physx::PxMaterial* GetMaterial() const
    {
      return material;
    }
    physx::PxDefaultCpuDispatcher* GetDispatcher() const
    {
      return dispatcher;
    }
    physx::PxPvd* GetPvd() const
    {
      return pvd;
    }
    physx::PxPvdTransport* GetTransport() const
    {
      return transport;
    }
    physx::PxFoundation* GetFoundation() const
    {
      return foundation;
    }

    void Cleanup()
    {
      if (material) {
        material->release();
        material = nullptr;
      }

      if (dispatcher) {
        dispatcher->release();
        dispatcher = nullptr;
      }

      if (physics) {
        physics->release();
        physics = nullptr;
      }

      if (transport) {
        transport->release();
        transport = nullptr;
      }

      if (pvd) {
        pvd->release();
        pvd = nullptr;
      }

      if (foundation) {
        foundation->release();
        foundation = nullptr;
      }
    }

    PhysicsResources(const PhysicsResources&) = delete;
    PhysicsResources& operator=(const PhysicsResources&) = delete;
  private:
    PhysicsResources()
    {
      foundation = PxCreateFoundation(PX_PHYSICS_VERSION, defaultAllocator, errorCallback);
      if (!foundation) {
        throw std::runtime_error("Failed to create PxFoundation");
      }

      pvd = PxCreatePvd(*foundation);
      if (!pvd) {
        throw std::runtime_error("Failed to create PxPvd");
      }

      transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
      if (!transport) {
        throw std::runtime_error("Failed to create PxPvdTransport");
      }

      pvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

      physx::PxTolerancesScale scale;
      physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, scale, true, pvd);
      if (!physics) {
        throw std::runtime_error("Failed to create PxPhysics");
      }

      dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
      if (!dispatcher) {
        throw std::runtime_error("Failed to create PxDefaultCpuDispatcher");
      }

      material = physics->createMaterial(0.5f, 0.5f, 0.1f);
      if (!material) {
        throw std::runtime_error("Failed to create PxMaterial");
      }
    }

    ~PhysicsResources() override = default;

    physx::PxFoundation*           foundation = nullptr;
    physx::PxPhysics*              physics = nullptr;
    physx::PxDefaultCpuDispatcher* dispatcher = nullptr;
    physx::PxMaterial*             material = nullptr;
    physx::PxPvd*                  pvd = nullptr;
    physx::PxPvdTransport*         transport = nullptr;
    physx::PxDefaultAllocator      defaultAllocator;
    physx::PxDefaultErrorCallback  errorCallback;
  };
}
