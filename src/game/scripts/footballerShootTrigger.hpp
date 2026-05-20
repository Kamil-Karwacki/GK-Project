#pragma once

#include "glm/ext/vector_float3.hpp"
#include "world/behaviour.hpp"
#include "world/entity.hpp"

class FootballerShootTrigger : public Behaviour
{
  public:
    FootballerShootTrigger(Entity *footballer) : m_footballer(footballer) {}
    void onUpdate(float deltaTime) override;
    void onTriggerEnter(Collider *otherCollider) override;
    virtual ~FootballerShootTrigger() override = default;

  private:
    Entity *m_footballer;
};
