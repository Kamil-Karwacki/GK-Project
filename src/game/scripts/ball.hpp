#pragma once

#include "world/behaviour.hpp"

class Footballer;

class Ball : public Behaviour
{
  public:
    void onUpdate(float deltaTime) override;
    void onCollisionEnter(Collider* otherCollider) override;

    void setLastContactFootballer(Footballer* footballer) { m_lastContactFootballer = footballer; }
    Footballer* getLastContactFootballer() const { return m_lastContactFootballer; }

  private:
    Footballer* m_lastContactFootballer = nullptr;
};

