#include "playerController.hpp"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/application.hpp"
#include "scripts/footballer.hpp"
#include "world/components/rigidbody.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"

void PlayerController::onStart()
{
    Transform *transform = m_entity->GetComponent<Transform>();

    if (!transform)
        std::cerr << "Error: PlayerController requires transform component!\n";
}

void PlayerController::onUpdate(float deltaTime)
{
    Footballer *footballer = m_entity->GetComponent<Footballer>();
    if (!footballer)
        return;

    Application &app = Application::Get();
    InputManager manager = app.GetInput();

    if (manager.isActionHeld("forward"))
    {
        footballer->m_input.y = 1;
    }
    if (manager.isActionHeld("back"))
    {
        footballer->m_input.y = -1;
    }
    if (manager.isActionHeld("left"))
    {
        footballer->m_input.x = -1;
    }
    if (manager.isActionHeld("right"))
    {
        footballer->m_input.x = 1;
    }
    if (manager.isActionPressed("shoot"))
    {
        footballer->kickBall();
    }
    if (manager.isActionPressed("jump"))
    {
        footballer->m_jump = true;
    }

    float mouseDeltaX = static_cast<float>(manager.getMouseDeltaX());
    float mouseDeltaY = static_cast<float>(manager.getMouseDeltaY());

    float mouseSensitivity = 0.003f;
    m_yaw += mouseDeltaX * mouseSensitivity;
    m_pitch += mouseDeltaY * mouseSensitivity;
    if (m_pitch > 1.2f)
        m_pitch = 1.2f;
    if (m_pitch < -1.2f)
        m_pitch = -1.2f;

    footballer->m_rotation = glm::vec2(-m_pitch, m_yaw);
}
