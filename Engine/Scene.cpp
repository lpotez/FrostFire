#include "Scene.h"

namespace FrostFireEngine
{
  Scene::~Scene() = default;

  World& Scene::GetWorld()
  {
    return world;
  }

  void Scene::Update(float deltaTime)
  {
    world.Update(deltaTime);
  }
}
