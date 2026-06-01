#include "matchArena.hpp"
#include "world/components/transform.hpp"

MatchArena::MatchArena(int id) : m_arenaID(id)
{
    m_physics = std::make_unique<PhysicsSystem>();
    float offsetX = (m_arenaID % 10) * 150.0f;
    float offsetZ = (static_cast<float>(m_arenaID) / 10) * 200.0f;
    m_arenaOffset = {offsetX, 0, offsetZ};
}

Entity &MatchArena::createEntity(Scene *scene)
{
    auto entity = std::make_unique<Entity>(scene);
    Entity *rawPtr = entity.get();
    m_entities.push_back(std::move(entity));
    return *rawPtr;
}

void MatchArena::resetPositions()
{
    m_playerA->GetComponent<Transform>()->setPosition(m_arenaOffset +
                                                      glm::vec3(0, 2, 50));
    m_playerA->GetComponent<Rigidbody>()->m_velocity = glm::vec3(0);
    m_playerA->GetComponent<Rigidbody>()->m_angularVelocity = glm::vec3(0);

    m_playerB->GetComponent<Transform>()->setPosition(m_arenaOffset +
                                                      glm::vec3(0, 2, -50));
    m_playerB->GetComponent<Rigidbody>()->m_velocity = glm::vec3(0);
    m_playerB->GetComponent<Rigidbody>()->m_angularVelocity = glm::vec3(0);

    m_ball->GetComponent<Transform>()->setPosition(m_arenaOffset +
                                                   glm::vec3(0, 5, 0));
    m_ball->GetComponent<Rigidbody>()->m_velocity = glm::vec3(0);

    m_needsReset = false;
    m_framesSinceLastReset = 0;
    glm::vec3 ballStart = m_arenaOffset + glm::vec3(0, 5, 0);
    m_prevBallToGateA = glm::distance(ballStart, m_gateAPos);
    m_prevBallToGateB = glm::distance(ballStart, m_gateBPos);

    m_prevAgentAToBall = glm::distance(
        m_playerA->GetComponent<Transform>()->getPosition(), ballStart);
    m_prevAgentBToBall = glm::distance(
        m_playerB->GetComponent<Transform>()->getPosition(), ballStart);
}

void MatchArena::fixedUpdate(float deltaTime)
{
    m_physics->update(m_entities, deltaTime);
    m_physics->generateContacts(m_entities);
    m_physics->resolveContacts(deltaTime);
}
