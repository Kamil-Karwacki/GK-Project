#include "baseScene.hpp"
#include "graphics/renderSystem.hpp"
#include "physics/physicsSystem.hpp"
#include "world/behaviour.hpp"
#include "world/components/transform.hpp"
#ifndef HEADLESS_MODE
#include "core/application.hpp"
#endif

BaseScene::BaseScene(unsigned int whiteTextureId)
{
    m_renderSystem = std::make_unique<RenderSystem>(whiteTextureId);
}

BaseScene::~BaseScene() = default;

void BaseScene::init() {}

void BaseScene::update(float deltaTime)
{
    for (Behaviour *behaviour : m_activeBehaviours)
    {
        behaviour->onUpdate(deltaTime);
    }
}

void BaseScene::fixedUpdate(float deltaTime)
{
    m_physicsSystem->update(m_entities, deltaTime);
    m_physicsSystem->generateContacts(m_entities);
    m_physicsSystem->resolveContacts(deltaTime);
}

void BaseScene::draw()
{
    m_renderSystem->render(m_entities, m_mainCamera->m_entity);
}

glm::mat4 BaseScene::getMainViewMatrix() const
{
    return m_mainCamera
               ? glm::inverse(m_mainCamera->m_entity->GetComponent<Transform>()
                                  ->getModelMatrix())
               : glm::mat4(1.0f);
}

glm::mat4 BaseScene::getMainProjectionMatrix() const
{
    if (m_mainCamera) {
#ifndef HEADLESS_MODE
        Application& app = Application::Get();
        float aspectRatio = (float)app.getWindowWidth() / (float)app.getWindowHeight();
        m_mainCamera->aspectRatio = aspectRatio;
#endif
        return m_mainCamera->getProjection();
    }
    return glm::mat4(1.0f);
}
