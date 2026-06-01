#pragma once
#include "world/behaviour.hpp"

class FreeCameraController : public Behaviour
{
  public:
    float m_speed = 150.0f;
    float m_sensitivity = 0.1f;

    void onStart() override;
    void onUpdate(float deltaTime) override;

  private:
    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
};
