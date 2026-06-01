#pragma once

#include "world/behaviour.hpp"

class Ball : public Behaviour
{
  public:
    void onUpdate(float deltaTime) override;
};
