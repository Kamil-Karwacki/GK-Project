#include "ball.hpp"
#include "scripts/footballer.hpp"
#include "world/entity.hpp"

void Ball::onUpdate(float deltaTime)
{
}

void Ball::onCollisionEnter(Collider* otherCollider)
{
    if (!otherCollider || !otherCollider->m_entity)
        return;

    Footballer* footballer = otherCollider->m_entity->GetComponent<Footballer>();
    if (footballer)
    {
        m_lastContactFootballer = footballer;
    }
}

