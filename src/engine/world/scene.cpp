#include "scene.hpp"

#include "physics/physicsSystem.hpp"
#include "world/entity.hpp"

Scene::Scene() { m_physicsSystem = std::make_unique<PhysicsSystem>(); }

Scene::~Scene() = default;

void Scene::init() {}

void Scene::update(float deltaTime)
{
    for (Behaviour *behaviour : m_activeBehaviours)
    {
        behaviour->onUpdate(deltaTime);
    }
}

void Scene::fixedUpdate(float deltaTime)
{
    m_physicsSystem->update(m_entities, deltaTime);
    m_physicsSystem->generateContacts(m_entities);
    m_physicsSystem->resolveContacts(deltaTime);
}

Entity &Scene::createEntity()
{
    std::unique_ptr<Entity> entity = std::make_unique<Entity>(this);

    Entity *rawPtr = entity.get();
    m_entities.push_back(std::move(entity));

    return *rawPtr;
}

void Scene::draw() {}

void Scene::addBehaviour(Behaviour *behaviour)
{
    m_activeBehaviours.push_back(behaviour);
}
