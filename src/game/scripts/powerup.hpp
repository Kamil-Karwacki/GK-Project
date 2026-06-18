#pragma once

#include "world/behaviour.hpp"
#include <glm/glm.hpp>

class Footballer;

enum class PowerupType
{
    SpeedBoost,
    SpeedDebuff,
    SuperJump,
    LowJump,
    SuperKick,
    WeakKick
};

class Powerup : public Behaviour
{
  public:
    Powerup(PowerupType type = PowerupType::SpeedBoost, float duration = 5.0f, float respawnTime = 8.0f);
    virtual ~Powerup() override = default;

    void onStart() override;
    void onUpdate(float deltaTime) override;
    void onTriggerEnter(Collider *otherCollider) override;

    void applyEffect(Footballer *footballer);
    void revertEffect();
    void moveToRandomPosition();
    float getRandomRespawnTime();
    void setTargetEntity(Entity *target) { m_targetEntity = target; }

  private:
    PowerupType m_type;
    float m_duration;
    float m_respawnTime;

    bool m_isActive = true;
    float m_buffTimer = 0.0f;
    float m_respawnTimer = 0.0f;

    Footballer *m_buffedFootballer = nullptr;
    glm::vec3 m_originalScale = glm::vec3(1.0f);
    Entity *m_targetEntity = nullptr;
};
