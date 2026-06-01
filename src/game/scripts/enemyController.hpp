#pragma once

#include "scripts/ai/math.hpp"
#include "world/behaviour.hpp"

class EnemyController : public Behaviour
{
  public:
    Entity *m_opponent;
    Entity *m_ball;

    glm::vec3 m_ownGatePos;
    glm::vec3 m_enemyGatePos;

    float m_yaw = 0.0f;
    float m_pitch = 0.0f;

    std::function<void(float)> onKickReward;

    void onStart() override;
    void onUpdate(float deltaTime) override;
    virtual ~EnemyController() override = default;
    void init(Entity *opponent, Entity *ball, glm::vec3 ownGatePos,
              glm::vec3 enemyGatePos);

  private:
    Matrix m_inputMatrix{38, 1};
    int m_frameCounter = 0;
    const int m_frameSkip = 4;

    float m_lastMoveX = 0.0f;
    float m_lastMoveY = 0.0f;
    float m_lastTurnYaw = 0.0f;
    float m_lastTurnPitch = 0.0f;
    bool m_lastJump = false;
    bool m_lastKick = false;
};
