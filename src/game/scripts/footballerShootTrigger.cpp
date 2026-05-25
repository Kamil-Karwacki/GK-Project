#include "footballerShootTrigger.hpp"
#include "scripts/footballer.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"
void FootballerShootTrigger::onUpdate(float deltaTime)
{
    if (!m_footballer)
        return;

    Transform *footballerTrans = m_footballer->GetComponent<Transform>();
    if (!footballerTrans)
        return;

    Transform *trans = m_entity->GetComponent<Transform>();
    if (!trans)
        return;

    trans->setPosition(footballerTrans->getPosition());
    trans->setRotation(footballerTrans->getRotation());
}

void FootballerShootTrigger::onTriggerEnter(Collider *otherCollider)
{
    Footballer *footballer = m_footballer->GetComponent<Footballer>();
    if (!footballer)
        return;

    Ball *ball = otherCollider->m_entity->GetComponent<Ball>();
    if (!ball)
        return;

    footballer->m_ball = ball;
}
