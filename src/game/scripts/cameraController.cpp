#include "cameraController.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"

void CameraController::onUpdate(float deltaTime)
{
    Transform *cameraTransform = m_entity->GetComponent<Transform>();
    Transform *playerTransform = m_playerEntity->GetComponent<Transform>();

    if (!cameraTransform || !playerTransform)
        return;

    cameraTransform->setPosition(playerTransform->getPosition() -
                                 playerTransform->getFront() * m_distToPlayer +
                                 m_offset);
    cameraTransform->lookAt(playerTransform->getPosition());
}
