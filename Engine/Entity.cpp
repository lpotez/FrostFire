#include "stdafx.h"
#include "Entity.h"

#include "ComponentManager.h"
#include "TransformComponent.h"
#include "RendererComponent.h"

namespace PM3D
{
    template<typename T>
    T* Entity::GetComponent() {
        return ComponentManager::GetInstance().GetComponent<T>(id);
    }

    template<typename T>
    void Entity::AddComponent(const T& component) {
        ComponentManager::GetInstance().AddComponent<T>(id, component);
    }

    template<typename T>
    void Entity::RemoveComponent() {
        ComponentManager::GetInstance().RemoveComponent<T>(id);
    }

    template<typename T>
    bool Entity::HasComponent() const {
        return ComponentManager::GetInstance().HasComponent<T>(id);
    }
}
