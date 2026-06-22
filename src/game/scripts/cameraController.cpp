#include "cameraController.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"

void CameraController::onUpdate(float deltaTime)
{
    Transform *cameraTransform = m_entity->GetComponent<Transform>();
    Transform *playerTransform = m_playerEntity->GetComponent<Transform>();

    if (!cameraTransform || !playerTransform)
        return;

    glm::vec3 desiredPos = playerTransform->getPosition() -
                           playerTransform->getFront() * m_distToPlayer +
                           m_offset;
                           
    if (desiredPos.y < 0.5f) {
        desiredPos.y = 0.5f;
    }

    cameraTransform->setPosition(desiredPos);
    cameraTransform->lookAt(playerTransform->getPosition() + glm::vec3(0, 2.0f, 0));
}
