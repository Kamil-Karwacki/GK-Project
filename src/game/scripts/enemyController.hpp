#pragma once

#include "world/behaviour.hpp"

class EnemyController : public Behaviour
{
  public:
    void onStart() override;
    void onUpdate(float deltaTime) override;
    virtual ~EnemyController() override = default;
};
