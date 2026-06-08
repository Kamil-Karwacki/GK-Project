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

    float m_speed = 30.0f;
    float m_jumpHeight = 1550.0f;
    float m_kickStrength = 5000.0f;
    Entity *m_shoe = nullptr;

  private:
    void kickLoop();
    bool m_shouldKick = false;
    float m_kickTimer = 0.0f;
};
