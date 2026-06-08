#include "matchArena.hpp"
#include "world/components/rigidbody.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"
#include <algorithm>

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
    m_playerA->GetComponent<Transform>()->setRotation(glm::vec3(0.0f));
    m_playerA->GetComponent<Rigidbody>()->m_velocity = glm::vec3(0);
    m_playerA->GetComponent<Rigidbody>()->m_angularVelocity = glm::vec3(0);

    m_playerB->GetComponent<Transform>()->setPosition(m_arenaOffset +
                                                      glm::vec3(0, 2, -50));
    m_playerB->GetComponent<Transform>()->setRotation(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
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

void MatchArena::onGoalA()
{
    m_fitnessB += 1000.0f;
    m_fitnessA -= 1000.0f;
    m_needsReset = true;
}

void MatchArena::onGoalB()
{
    m_fitnessA += 1000.0f;
    m_fitnessB -= 1000.0f;
    m_needsReset = true;
}

void MatchArena::onKickA(float reward)
{
    if (reward > 0.0f)
        m_fitnessA += reward;
    else
        m_fitnessA -= 50.0f;
}

void MatchArena::onKickB(float reward)
{
    if (reward > 0.0f)
        m_fitnessB += reward;
    else
        m_fitnessB -= 50.0f;
}

void MatchArena::updateFitness()
{
    Transform *transA = m_playerA->GetComponent<Transform>();
    Transform *transB = m_playerB->GetComponent<Transform>();
    Transform *ballTrans = m_ball->GetComponent<Transform>();

    glm::vec3 ballPos = ballTrans->getPosition();
    glm::vec3 agentAPos = transA->getPosition();
    glm::vec3 agentBPos = transB->getPosition();

    float currBallToGateB = glm::distance(ballPos, m_gateBPos);
    float deltaBallA = m_prevBallToGateB - currBallToGateB;
    m_fitnessA += deltaBallA * 4.0f;
    m_prevBallToGateB = currBallToGateB;

    glm::vec3 agentAToBall = glm::normalize(ballPos - agentAPos);
    glm::vec3 agentAToGateB = glm::normalize(m_gateBPos - agentAPos);
    float shootAlignA = glm::dot(agentAToBall, agentAToGateB);
    if (shootAlignA > 0.7f)
        m_fitnessA += shootAlignA * 0.1f;

    m_fitnessA -= 0.01f;

    float currBallToGateA = glm::distance(ballPos, m_gateAPos);
    float deltaBallB = m_prevBallToGateA - currBallToGateA;
    m_fitnessB += deltaBallB * 4.0f;
    m_prevBallToGateA = currBallToGateA;

    glm::vec3 agentBToBall = glm::normalize(ballPos - agentBPos);
    glm::vec3 agentBToGateA = glm::normalize(m_gateAPos - agentBPos);
    float shootAlignB = glm::dot(agentBToBall, agentBToGateA);
    if (shootAlignB > 0.7f)
        m_fitnessB += shootAlignB * 0.1f;

    m_fitnessB -= 0.01f;
}

void MatchArena::step(float deltaTime, float gravity)
{
    for (auto &entity : m_entities)
    {
        Rigidbody *rb = entity->GetComponent<Rigidbody>();
        if (rb)
        {
            rb->m_forceAcc += glm::vec3(0.0f, -1.0f, 0.0f) * gravity *
                              (1.0f / rb->m_inverseMass);
        }
    }

    fixedUpdate(deltaTime);
    m_framesSinceLastReset++;

    updateFitness();

    if (m_needsReset || m_framesSinceLastReset > 1800)
    {
        resetPositions();
    }
}
