#include "scene.hpp"

#include "graphics/renderSystem.hpp"
#include "physics/physicsSystem.hpp"
#include "world/components/transform.hpp"

Scene::Scene(unsigned int whiteTextureId)
{
    m_renderSystem = std::make_unique<RenderSystem>(whiteTextureId);
    m_physicsSystem = std::make_unique<PhysicsSystem>();
}

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

void Scene::draw()
{
    m_renderSystem->render(m_entities, m_mainCamera->m_entity);
}

glm::mat4 Scene::getMainViewMatrix() const
{
    return m_mainCamera
               ? glm::inverse(m_mainCamera->m_entity->GetComponent<Transform>()
                                  ->getModelMatrix())
               : glm::mat4(1.0f);
}

glm::mat4 Scene::getMainProjectionMatrix() const
{
    return m_mainCamera ? m_mainCamera->getProjection() : glm::mat4(1.0f);
}

Entity &Scene::createEntity()
{
    std::unique_ptr<Entity> entity = std::make_unique<Entity>(this);

    Entity *rawPtr = entity.get();
    m_entities.push_back(std::move(entity));

    return *rawPtr;
}

void Scene::addBehaviour(Behaviour *behaviour)
{
    m_activeBehaviours.push_back(behaviour);
}
