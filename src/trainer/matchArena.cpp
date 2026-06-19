#include "matchArena.hpp"
#include "scripts/enemyController.hpp"
#include "world/components/rigidbody.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"

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
    if (auto ecA = m_playerA->GetComponent<EnemyController>())
    {
        ecA->m_yaw = ecA->m_initialYaw;
        ecA->m_pitch = 0.0f;
    }

    m_playerB->GetComponent<Transform>()->setPosition(m_arenaOffset +
                                                      glm::vec3(0, 2, -50));
    m_playerB->GetComponent<Transform>()->setRotation(
        glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
    m_playerB->GetComponent<Rigidbody>()->m_velocity = glm::vec3(0);
    m_playerB->GetComponent<Rigidbody>()->m_angularVelocity = glm::vec3(0);
    if (auto ecB = m_playerB->GetComponent<EnemyController>())
    {
        ecB->m_yaw = ecB->m_initialYaw;
        ecB->m_pitch = 0.0f;
    }

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

void MatchArena::onKickA(float reward) { m_fitnessA += reward; }

void MatchArena::onKickB(float reward) { m_fitnessB += reward; }

void MatchArena::updateFitness()
{
    Transform *transA = m_playerA->GetComponent<Transform>();
    Transform *transB = m_playerB->GetComponent<Transform>();
    Transform *ballTrans = m_ball->GetComponent<Transform>();
    Rigidbody *ballRb = m_ball->GetComponent<Rigidbody>();

    glm::vec3 ballPos = ballTrans->getPosition();
    glm::vec3 agentAPos = transA->getPosition();
    glm::vec3 agentBPos = transB->getPosition();

    float currDistToBallA = glm::distance(agentAPos, ballPos);
    float deltaDistToBallA = m_prevAgentAToBall - currDistToBallA;
    m_fitnessA += deltaDistToBallA * 2.0f;
    m_prevAgentAToBall = currDistToBallA;

    float currDistToBallB = glm::distance(agentBPos, ballPos);
    float deltaDistToBallB = m_prevAgentBToBall - currDistToBallB;
    m_fitnessB += deltaDistToBallB * 2.0f;
    m_prevAgentBToBall = currDistToBallB;

    float currBallToGateB = glm::distance(ballPos, m_gateBPos);
    float deltaBallA = m_prevBallToGateB - currBallToGateB;
    m_fitnessA += deltaBallA * 4.0f;
    m_prevBallToGateB = currBallToGateB;

    glm::vec3 agentAToBall = glm::normalize(ballPos - agentAPos);
    glm::vec3 agentAToGateB = glm::normalize(m_gateBPos - agentAPos);
    float shootAlignA = glm::dot(agentAToBall, agentAToGateB);
    if (shootAlignA > 0.7f)
        m_fitnessA += shootAlignA * 0.1f;

    glm::vec3 agentAForward = transA->getFront();
    float facingBallA = glm::dot(agentAForward, agentAToBall);
    m_fitnessA += facingBallA * 0.005f;

    glm::vec3 ballVel = ballRb ? ballRb->m_velocity : glm::vec3(0.0f);
    if (glm::length(ballVel) > 0.0001f)
    {
        glm::vec3 ballDir = glm::normalize(ballVel);
        glm::vec3 ballToGateB = glm::normalize(m_gateBPos - ballPos);
        float ballVelAlignA = glm::dot(ballDir, ballToGateB);
        if (ballVelAlignA > 0.0f)
            m_fitnessA += ballVelAlignA * glm::length(ballVel) * 0.01f;

        glm::vec3 ballToGateA = glm::normalize(m_gateAPos - ballPos);
        float ballVelAlignB = glm::dot(ballDir, ballToGateA);
        if (ballVelAlignB > 0.0f)
            m_fitnessB += ballVelAlignB * glm::length(ballVel) * 0.01f;
    }

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

    glm::vec3 agentBForward = transB->getFront();
    float facingBallB = glm::dot(agentBForward, agentBToBall);
    m_fitnessB += facingBallB * 0.005f;

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
