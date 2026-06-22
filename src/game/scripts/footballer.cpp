#include "footballer.hpp"

#include "glm/geometric.hpp"
#include "scripts/shoeController.hpp"
#include "world/components/rigidbody.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"

const CharacterDef CHARACTERS[3] = {
    {"Fast player", "assets/models/footballer1.obj",
     Footballer::BASE_SPEED * 1.3f, Footballer::BASE_JUMP_HEIGHT,
     Footballer::BASE_KICK_STRENGTH * 0.8f},
    {"Strong player", "assets/models/footballer2.obj",
     Footballer::BASE_SPEED * 0.8f, Footballer::BASE_JUMP_HEIGHT * 1.1f,
     Footballer::BASE_KICK_STRENGTH * 1.5f},
    {"Balanced", "assets/models/model.obj", Footballer::BASE_SPEED,
     Footballer::BASE_JUMP_HEIGHT, Footballer::BASE_KICK_STRENGTH}};

void Footballer::onUpdate(float deltaTime)
{
    m_kickTimer -= deltaTime;
    kickLoop();
    move(deltaTime);
}

void Footballer::kickBall() { m_shouldKick = true; }

void Footballer::kickLoop()
{
    if (!m_shouldKick)
        return;
    m_shouldKick = false;

    if (m_kickTimer > 0)
        return;

    ShoeController *shoe = nullptr;
    if (m_shoe)
        shoe = m_shoe->GetComponent<ShoeController>();
    if (shoe)
        shoe->m_isKicking = true;

    if (!m_ball)
    {
        if (m_onKickCallback)
            m_onKickCallback(false, glm::vec3(0.0f));
        m_kickTimer = 0.5f;
        return;
    }
    Rigidbody *ballRb = m_ball->m_entity->GetComponent<Rigidbody>();
    if (!ballRb)
        return;
    Transform *ballTrans = m_ball->m_entity->GetComponent<Transform>();
    if (!ballTrans)
        return;

    Transform *transform = m_entity->GetComponent<Transform>();

    glm::vec3 kickDir = transform->getFront();
    kickDir.y += 0.2f; // slight upward lift
    kickDir = glm::normalize(kickDir);

    float distToBall =
        glm::distance(transform->getPosition(), ballTrans->getPosition());

    static constexpr float maxKickDistance = 6.0f;
    if (distToBall > maxKickDistance)
    {
        if (m_onKickCallback)
            m_onKickCallback(false, glm::vec3(0.0f));
        m_ball = nullptr;
        m_kickTimer = 0.5f;
        return;
    }
    float kickModifier = 8.0f / pow(distToBall, 1.5f);
    ballRb->m_forceAcc += m_kickStrength * kickDir * kickModifier;

    m_ball->setLastContactFootballer(this);

    if (m_onKickCallback)
        m_onKickCallback(true, kickDir);

    m_ball = nullptr;
    m_kickTimer = 0.5f;
}
#define Lerp(current, target, t) (current + (target - current) * t)
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

    glm::vec3 targetVel = glm::cross(front, up) * m_speed * m_input.x +
                          front * m_speed * m_input.y;

    static constexpr float accelerationRate = 12.0f;
    static constexpr float decelerationRate = 20.0f;

    bool isMoving = (targetVel.x != 0.0f || targetVel.z != 0.0f);

    float currentRate = isMoving ? accelerationRate : decelerationRate;

    float t = currentRate * deltaTime;
    if (t > 1.0f)
        t = 1.0f;

    float newX = Lerp(rigidbody->m_velocity.x, targetVel.x, t);
    float newZ = Lerp(rigidbody->m_velocity.z, targetVel.z, t);
    rigidbody->m_velocity.x = newX;
    rigidbody->m_velocity.z = newZ;

    if (m_groundTimer > 0.0f && m_jump && rigidbody->m_velocity.y < 0.5f)
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
