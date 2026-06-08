#pragma once

#include "world/behaviour.hpp"
#include "world/entity.hpp"
class ShoeController : public Behaviour
{
  public:
    glm::vec3 m_offset{0, -1, 0};
    float m_moveTimer = 0.0f;
    bool m_isKicking = false;
    Entity *m_player;

    ShoeController(glm::vec3 offset, Entity *player)
        : m_offset(offset), m_player(player)
    {
    }
    void onUpdate(float deltaTime) override;
    virtual ~ShoeController() override = default;
};
