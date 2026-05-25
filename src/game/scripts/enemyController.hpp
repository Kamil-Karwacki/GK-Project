#pragma once

#include "world/behaviour.hpp"

class EnemyController : public Behaviour
{
  public:
    Entity *m_opponent;
    Entity *m_ball;
    glm::vec3 m_ownGatePos;
    glm::vec3 m_enemyGatePos;
    std::function<void(float)> onKickReward;

    void onStart() override;
    void onUpdate(float deltaTime) override;
    virtual ~EnemyController() override = default;
    void init(Entity *opponent, Entity *ball, glm::vec3 ownGatePos,
              glm::vec3 enemyGatePos);
};
