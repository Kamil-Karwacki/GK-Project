#include "gateTrigger.hpp"
#include "scripts/ball.hpp"
#include "world/entity.hpp"

void GateTrigger::onTriggerEnter(Collider *otherCollider)
{
    if (otherCollider->m_entity->GetComponent<Ball>() != nullptr)
    {
        if (m_onGoalScored)
        {
            m_onGoalScored();
        }
    }
}
