#pragma once
#include <glm/glm.hpp>

#include "world/behaviour.hpp"

class PlayerController : public Behaviour
{
   public:
    void onStart() override;
    void onUpdate(float deltaTime) override;
    void resetRotation(float yaw = 0.0f, float pitch = 0.0f);
    virtual ~PlayerController() override = default;

   private:
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
};
