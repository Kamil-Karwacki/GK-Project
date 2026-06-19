#pragma once
#include <glm/glm.hpp>
#include <functional>

#include "scripts/ball.hpp"
#include "world/behaviour.hpp"

struct CharacterDef {
    const char* name;
    const char* modelPath;
    float speed;
    float jumpHeight;
    float kickStrength;
};
extern const CharacterDef CHARACTERS[3];

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
    std::function<void(bool, glm::vec3)> m_onKickCallback;

    static constexpr float BASE_SPEED = 30.0f;
    static constexpr float BASE_JUMP_HEIGHT = 1550.0f;
    static constexpr float BASE_KICK_STRENGTH = 5000.0f;

    float m_speed = BASE_SPEED;
    float m_jumpHeight = BASE_JUMP_HEIGHT;
    float m_kickStrength = BASE_KICK_STRENGTH;
    Entity *m_shoe = nullptr;

  private:
    void kickLoop();
    bool m_shouldKick = false;
    float m_kickTimer = 0.0f;
};
