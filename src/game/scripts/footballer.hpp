#pragma once
#include <glm/glm.hpp>

#include "scripts/ball.hpp"
#include "world/behaviour.hpp"

class Footballer : public Behaviour
{
  public:
    void onUpdate(float deltaTime) override;

    void kickBall();
    void move(float deltaTime);

    glm::vec2 m_input = glm::vec2(0.0f);
    glm::vec2 m_rotation = glm::vec2(0.0f);
    bool m_jump = false;
    float m_groundTimer = 0.1f;
    Ball *m_ball = nullptr;

  private:
    void kickLoop();
    bool m_shouldKick = false;
    float m_kickStrength = 170.0f;
    float m_speed = 250.0f;
    float m_jumpHeight = 110.0f;
};
