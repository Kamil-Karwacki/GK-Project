#include "footballer.hpp"

#include "glm/geometric.hpp"
#include "world/components/rigidbody.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"

void Footballer::onUpdate(float deltaTime)
{
    kickLoop();
    move(deltaTime);
}

void Footballer::kickBall() { m_shouldKick = true; }

void Footballer::kickLoop()
{
    if (!m_shouldKick)
        return;
    m_shouldKick = false;
    if (!m_ball)
        return;
    Rigidbody *ballRb = m_ball->m_entity->GetComponent<Rigidbody>();
    if (!ballRb)
        return;
    Transform *ballTrans = m_ball->m_entity->GetComponent<Transform>();
    if (!ballTrans)
        return;

    Transform *transform = m_entity->GetComponent<Transform>();

    glm::vec3 front = transform->getFront();
    glm::vec3 kickDir = -front;
    front.y *= -1.2f;
    float distToBall =
        glm::distance(transform->getPosition(), ballTrans->getPosition());

    static constexpr float maxKickDistance = 6.0f;
    if (distToBall > maxKickDistance)
    {
        m_ball = nullptr;
        return;
    }
    float kickModifier = 8.0f / pow(distToBall, 1.5f);
    ballRb->m_forceAcc += m_kickStrength * kickDir * kickModifier;
    m_ball = nullptr;
}

void Footballer::move(float deltaTime)
{
    Transform *transform = m_entity->GetComponent<Transform>();
    Rigidbody *rigidbody = m_entity->GetComponent<Rigidbody>();

    if (!transform || !rigidbody)
        return;

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    transform->setRotation(glm::vec3(0.0f, -m_rotation.y, 0.0f));
    glm::vec3 front = transform->getFront();

    transform->setRotation(glm::vec3(-m_rotation.x, -m_rotation.y, 0.0f));

    rigidbody->m_forceAcc +=
        -front * deltaTime * m_speed * rigidbody->getMass() * m_input.y;
    rigidbody->m_forceAcc += -glm::cross(front, up) * deltaTime * m_speed *
                             rigidbody->getMass() * m_input.x;

    if (m_groundTimer > 0.0f && m_jump)
    {
        rigidbody->m_forceAcc +=
            glm::vec3(0, m_jumpHeight * rigidbody->getMass(), 0);
        m_groundTimer = 0.0f;
    }

    m_input = glm::vec2(0.0f, 0.0f);
    m_rotation = glm::vec2(0.0f);
    m_jump = false;
    m_groundTimer -= deltaTime;
}
