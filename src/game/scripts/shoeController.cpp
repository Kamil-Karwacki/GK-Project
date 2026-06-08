#include "shoeController.hpp"
#include "glm/trigonometric.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

void ShoeController::onUpdate(float deltaTime)
{
    Transform *trans = m_entity->GetComponent<Transform>();
    if (!trans)
        return;

    Transform *playerTrans = m_player->GetComponent<Transform>();
    if (!playerTrans)
        return;

    glm::vec3 front = playerTrans->getFront();
    front.y = 0.0f;

    if (glm::length(front) > 0.001f)
    {
        front = glm::normalize(front);
    }
    else
    {
        front = glm::vec3(0.0f, 0.0f, -1.0f);
    }

    float distanceForward = 1.8f;
    float distanceDown = -1.2f;

    glm::vec3 basePos = playerTrans->getPosition() + (front * distanceForward) +
                        glm::vec3(0.0f, distanceDown, 0.0f);

    if (!m_isKicking)
    {
        trans->setPosition(basePos);
    }
    else
    {
        m_moveTimer += deltaTime;
        static constexpr float timeToKick = 0.3f;

        if (m_moveTimer >= timeToKick)
        {
            m_isKicking = false;
            m_moveTimer = 0.0f;
            trans->setPosition(basePos);
        }
        else
        {
            float t = m_moveTimer / timeToKick;

            float arcForward = glm::sin(t * glm::pi<float>()) * 2.5f;
            float arcY = glm::sin(t * glm::pi<float>()) *
                         glm::sin(t * glm::pi<float>()) * 1.5f;

            glm::vec3 animatedPos =
                basePos + glm::vec3(0.0f, arcY, 0.0f) + (front * arcForward);
            trans->setPosition(animatedPos);
        }
    }

    trans->setRotation(playerTrans->getRotation());
    trans->addRotation({0, glm::radians(270.0f), 0});
}
