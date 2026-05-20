#include "playerGrounded.hpp"
#include "scripts/footballer.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"

void PlayerGrounded::onUpdate(float deltaTime)
{
    if (!m_player)
        return;
    Transform *playerTrans = m_player->GetComponent<Transform>();
    if (!playerTrans)
        return;
    Transform *trans = m_entity->GetComponent<Transform>();
    if (!trans)
        return;

    glm::vec3 playerPos = playerTrans->getPosition();
    trans->setPosition(playerPos + m_offset);
}

void PlayerGrounded::onTriggerEnter(Collider *other)
{
    Footballer *footballer = m_player->GetComponent<Footballer>();
    if (!footballer)
        return;

    footballer->m_groundTimer = 0.2f;
}
