#pragma once
#include <functional>
#include <unordered_map>
#include <string>
#include <memory>
#include <shared_mutex>
#include "MeshFactory.h"
#include "Singleton.h"

namespace FrostFireEngine
{
   // Gestionnaire de cache de meshes utilisant un shared_mutex pour l'accès concurrent
   // Les meshes sont stockés sous forme de shared_ptr pour la gestion auto de la mémoire
   class MeshManager final : public CSingleton<MeshManager>
   {
       friend class CSingleton<MeshManager>;

   public:
       // Cache le mesh en utilisant la taille comme paramètre de différenciation
       std::shared_ptr<Mesh> GetCubeMesh(ID3D11Device* device, float size = 1.0f)
       {
           if (!device) return nullptr;
           std::string key = "Cube_" + std::to_string(size);
           return GetOrCreateMesh(key, device, [device, size]() {
               return MeshFactory::CreateCube(device, size);
           });
       }

       std::shared_ptr<Mesh> GetCubeMapMesh(ID3D11Device* device, float size = 1.0f)
       {
           if (!device) return nullptr;
           std::string key = "CubeMap_" + std::to_string(size);
           return GetOrCreateMesh(key, device, [device, size]() {
               return MeshFactory::CreateCubeMap(device, size);
           });
       }

       // sectorCount/stackCount = résolution de la sphère
       // Valeurs élevées = plus lisse mais plus coûteux en mémoire
       std::shared_ptr<Mesh> GetSphereMesh(ID3D11Device* device, float radius = 1.0f,
                                         uint32_t sectorCount = 36, uint32_t stackCount = 18)
       {
           if (!device) return nullptr;
           std::string key = "Sphere_" + std::to_string(radius) + "_" +
                           std::to_string(sectorCount) + "_" + std::to_string(stackCount);
           return GetOrCreateMesh(key, device, [device, radius, sectorCount, stackCount]() {
               return MeshFactory::CreateSphere(device, radius, sectorCount, stackCount);
           });
       }

       // Crée un plan sur XZ avec Y vers le haut
       std::shared_ptr<Mesh> GetPlaneMesh(ID3D11Device* device, float width = 1.0f, float depth = 1.0f)
       {
           if (!device) return nullptr;
           std::string key = "Plane_" + std::to_string(width) + "_" + std::to_string(depth);
           return GetOrCreateMesh(key, device, [device, width, depth]() {
               return MeshFactory::CreatePlane(device, width, depth);
           });
       }

       // Capsule = cylindre + deux demi-sphères (height = hauteur du cylindre central)
       std::shared_ptr<Mesh> GetCapsuleMesh(ID3D11Device* device, float radius = 1.0f,
                                          float height = 2.0f, uint32_t sectorCount = 36)
       {
           if (!device) return nullptr;
           std::string key = "Capsule_" + std::to_string(radius) + "_" +
                           std::to_string(height) + "_" + std::to_string(sectorCount);
           return GetOrCreateMesh(key, device, [device, radius, height, sectorCount]() {
               return MeshFactory::CreateCapsule(device, radius, height, sectorCount);
           });
       }

     std::shared_ptr<Mesh> GetQuadMesh(ID3D11Device* device){
         if (!device) return nullptr;
         std::string key = "Quad";
         return GetOrCreateMesh(key, device, [device]() {
             return MeshFactory::CreateQuad(device);
         });
       }

       // Thread-safe : verrouillage exclusif du cache
       void Clear()
       {
           std::unique_lock<std::shared_mutex> lock(mutex_);
           meshCache_.clear();
       }

       // Thread-safe : lecture concurrente possible
       [[nodiscard]] bool HasMesh(const std::string& key) const
       {
           std::shared_lock<std::shared_mutex> lock(mutex_);
           return meshCache_.contains(key);
       }

       void RemoveMesh(const std::string& key)
       {
           std::unique_lock<std::shared_mutex> lock(mutex_);
           meshCache_.erase(key);
       }

       [[nodiscard]] size_t GetCacheSize() const
       {
           std::shared_lock<std::shared_mutex> lock(mutex_);
           return meshCache_.size();
       }

   protected:
       // Constructeur protégé car singleton
       MeshManager() = default;

   private:
       // Cache des meshes avec shared_ptr pour gestion auto de la durée de vie
       std::unordered_map<std::string, std::shared_ptr<Mesh>> meshCache_;

       // Mutex permettant lectures simultanées mais écriture exclusive
       mutable std::shared_mutex mutex_;

       // Double vérification pour optimiser les performances
       // 1- Check rapide en lecture partagée
       // 2- Si absent, check + création en écriture exclusive
       std::shared_ptr<Mesh> GetOrCreateMesh(const std::string& key,
                                           ID3D11Device* device,
                                           const std::function<std::shared_ptr<Mesh>()>& createFunc)
       {
           if (!device) return nullptr;

           // Premier check en lecture partagée
           {
               std::shared_lock<std::shared_mutex> readLock(mutex_);
               auto it = meshCache_.find(key);
               if (it != meshCache_.end()) {
                   return it->second;
               }
           }

           // Si non trouvé, verrou exclusif et second check
           std::unique_lock<std::shared_mutex> writeLock(mutex_);
           auto it = meshCache_.find(key);
           if (it != meshCache_.end()) {
               return it->second;
           }

           // Création uniquement si toujours absent
           std::shared_ptr<Mesh> newMesh = createFunc();
           if (newMesh) {
               meshCache_[key] = newMesh;
           }
           return newMesh;
       }
   };

} // namespace FrostFireEngine
