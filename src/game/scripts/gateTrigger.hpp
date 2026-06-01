#pragma once

#include "world/behaviour.hpp"

class GateTrigger : public Behaviour
{
  public:
    std::function<void()> m_onGoalScored;

    GateTrigger(std::function<void()> callback) : m_onGoalScored(callback) {}

    void onTriggerEnter(Collider *otherCollider) override;
    virtual ~GateTrigger() override = default;
};
