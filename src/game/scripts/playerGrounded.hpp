#pragma once

#include "world/behaviour.hpp"

class Entity;

class PlayerGrounded : public Behaviour
{
  public:
    void onUpdate(float deltaTime) override;
    PlayerGrounded(Entity *player, glm::vec3 offset)
        : m_offset(offset), m_player(player) {};
    virtual ~PlayerGrounded() override = default;
    void onTriggerEnter(Collider *other) override;
    glm::vec3 m_offset;

  private:
    Entity *m_player;
};
