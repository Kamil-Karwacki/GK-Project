#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "world/component.hpp"

class Camera : public Component
{
  public:
    virtual ~Camera() override = default;

    float fov = 45.0f;
    float nearPlane = 0.01f;
    float farPlane = 5000.0f;
    float aspectRatio = 16.0f / 9.0f;

    glm::mat4 getProjection() const
    {
        return glm::perspective(glm::radians(fov), aspectRatio, nearPlane,
                                farPlane);
    }
};
