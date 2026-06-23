#include "powerup.hpp"
#include "scripts/ball.hpp"
#include "scripts/footballer.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"
#include <random>

Powerup::Powerup(PowerupType type, float duration, float respawnTime)
    : m_type(type), m_duration(duration), m_respawnTime(respawnTime)
{
}

void Powerup::onStart()
{
    Transform *trans = m_entity->GetComponent<Transform>();
    if (trans)
    {
        m_originalScale = trans->getScale();
        trans->setScale(glm::vec3(0.0f));
    }
    m_isActive = false;
    m_respawnTimer = getRandomRespawnTime();
}

void Powerup::onUpdate(float deltaTime)
{
    if (m_buffedFootballer)
    {
        m_buffTimer -= deltaTime;
        if (m_buffTimer <= 0.0f)
        {
            revertEffect();
        }
    }

    if (!m_isActive)
    {
        m_respawnTimer -= deltaTime;
        if (m_respawnTimer <= 0.0f)
        {
            m_isActive = true;
            moveToRandomPosition();
            Transform *trans = m_entity->GetComponent<Transform>();
            if (trans)
            {
                trans->setScale(m_originalScale);
            }
        }
    }

    if (m_isActive && m_targetEntity)
    {
        Transform *trans = m_entity->GetComponent<Transform>();
        Transform *targetTrans = m_targetEntity->GetComponent<Transform>();
        if (trans && targetTrans)
        {
            glm::vec3 targetPos = targetTrans->getPosition();
            targetPos.y = trans->getPosition().y;
            trans->lookAt(targetPos);
        }
    }
}

void Powerup::onTriggerEnter(Collider *otherCollider)
{
    if (!m_isActive || !otherCollider || !otherCollider->m_entity)
        return;

    Ball *ball = otherCollider->m_entity->GetComponent<Ball>();
    if (ball)
    {
        Footballer *footballer = ball->getLastContactFootballer();
        if (footballer)
        {
            applyEffect(footballer);
        }
    }
}

void Powerup::applyEffect(Footballer *footballer)
{
    if (!footballer)
        return;

    // If this footballer is already buffed by this powerup, revert it first to
    // refresh the timer
    if (m_buffedFootballer)
    {
        revertEffect();
    }

    m_buffedFootballer = footballer;
    m_buffTimer = m_duration;
    m_isActive = false;

    Transform *trans = m_entity->GetComponent<Transform>();
    if (trans)
    {
        trans->setScale(glm::vec3(0.0f));
    }
    m_respawnTimer = getRandomRespawnTime();

    switch (m_type)
    {
    case PowerupType::SpeedBoost:
        footballer->m_speed *= 1.5f;
        break;
    case PowerupType::SpeedDebuff:
        footballer->m_speed *= 0.6f;
        break;
    case PowerupType::SuperJump:
        footballer->m_jumpHeight *= 1.4f;
        break;
    case PowerupType::LowJump:
        footballer->m_jumpHeight *= 0.6f;
        break;
    case PowerupType::SuperKick:
        footballer->m_kickStrength *= 1.8f;
        break;
    case PowerupType::WeakKick:
        footballer->m_kickStrength *= 0.5f;
        break;
    }
}

void Powerup::revertEffect()
{
    if (!m_buffedFootballer)
        return;

    switch (m_type)
    {
    case PowerupType::SpeedBoost:
        m_buffedFootballer->m_speed /= 1.5f;
        break;
    case PowerupType::SpeedDebuff:
        m_buffedFootballer->m_speed /= 0.6f;
        break;
    case PowerupType::SuperJump:
        m_buffedFootballer->m_jumpHeight /= 1.4f;
        break;
    case PowerupType::LowJump:
        m_buffedFootballer->m_jumpHeight /= 0.6f;
        break;
    case PowerupType::SuperKick:
        m_buffedFootballer->m_kickStrength /= 1.8f;
        break;
    case PowerupType::WeakKick:
        m_buffedFootballer->m_kickStrength /= 0.5f;
        break;
    }

    m_buffedFootballer = nullptr;
}

void Powerup::moveToRandomPosition()
{
    Transform *trans = m_entity->GetComponent<Transform>();
    if (!trans)
        return;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distX(-40.0f, 40.0f);
    std::uniform_real_distribution<float> distY(5.0f, 15.0f);
    std::uniform_real_distribution<float> distZ(-55.0f, 55.0f);

    glm::vec3 newPos(distX(gen), distY(gen), distZ(gen));
    trans->setPosition(newPos);
}

void Powerup::reset()
{
    if (m_buffedFootballer)
    {
        revertEffect();
    }
    m_isActive = false;
    m_buffTimer = 0.0f;
    m_respawnTimer = getRandomRespawnTime();

    Transform *trans = m_entity->GetComponent<Transform>();
    if (trans)
    {
        trans->setScale(glm::vec3(0.0f));
    }
}

float Powerup::getRandomRespawnTime()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(15.0f, 60.0f);
    return dist(gen);
}
