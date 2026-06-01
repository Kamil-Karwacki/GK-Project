#include "freeCameraController.hpp"
#include "core/application.hpp" // Upewnij się, że to tutaj masz dostęp do InputManagera
#include "world/components/transform.hpp"
#include "world/entity.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

void FreeCameraController::onStart() {}

void FreeCameraController::onUpdate(float deltaTime)
{
    Transform *trans = m_entity->GetComponent<Transform>();
    if (!trans)
        return;

    auto &input = Application::Get().GetInput();

    float xOffset = static_cast<float>(input.getMouseDeltaX());
    float yOffset = static_cast<float>(input.getMouseDeltaY());

    xOffset *= m_sensitivity;
    yOffset *= m_sensitivity;

    m_yaw += xOffset;
    m_pitch += yOffset;

    if (m_pitch > 89.0f)
        m_pitch = 89.0f;
    if (m_pitch < -89.0f)
        m_pitch = -89.0f;

    trans->setRotation(
        glm::vec3(glm::radians(m_pitch), glm::radians(-m_yaw), 0.0f));

    glm::vec3 pos = trans->getPosition();
    glm::vec3 front = trans->getFront();

    glm::vec3 right =
        glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    float velocity = m_speed * deltaTime;

    if (input.isKeyDown(GLFW_KEY_LEFT_SHIFT))
        velocity *= 3.0f;

    if (input.isKeyDown(GLFW_KEY_W))
        pos += front * velocity;
    if (input.isKeyDown(GLFW_KEY_S))
        pos -= front * velocity;
    if (input.isKeyDown(GLFW_KEY_A))
        pos -= right * velocity;
    if (input.isKeyDown(GLFW_KEY_D))
        pos += right * velocity;

    if (input.isKeyDown(GLFW_KEY_E))
        pos += up * velocity;
    if (input.isKeyDown(GLFW_KEY_Q))
        pos -= up * velocity;

    trans->setPosition(pos);
}
