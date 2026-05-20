#include "enemyController.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "scripts/footballer.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"

void EnemyController::onStart()
{
    Transform *transform = m_entity->GetComponent<Transform>();

    if (!transform)
        std::cerr << "Error: EnemyController requires transform component!\n";
}

void EnemyController::onUpdate(float deltaTime)
{
    Footballer *footballer = m_entity->GetComponent<Footballer>();
    if (!footballer)
        return;

    // ai code here
}
