#include "matchArena.hpp"
#include "scripts/enemyController.hpp"
#include "world/components/rigidbody.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"
#include <random>

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
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-10.0f, 10.0f);
    std::uniform_real_distribution<float> velDist(-5.0f, 5.0f);

    float ax = posDist(gen);
    float az = posDist(gen);
    float bx = posDist(gen);
    float bz = posDist(gen);
    float ballX = posDist(gen) * 1.5f;
    float ballZ = posDist(gen) * 1.5f;

    m_playerA->GetComponent<Transform>()->setPosition(
        m_arenaOffset + glm::vec3(ax, 2, 50 + az));
    m_playerA->GetComponent<Transform>()->setRotation(glm::vec3(0.0f));
    m_playerA->GetComponent<Rigidbody>()->m_velocity = glm::vec3(0);
    m_playerA->GetComponent<Rigidbody>()->m_angularVelocity = glm::vec3(0);
    if (auto ecA = m_playerA->GetComponent<EnemyController>())
    {
        ecA->m_yaw = ecA->m_initialYaw;
        ecA->m_pitch = 0.0f;
    }

    m_playerB->GetComponent<Transform>()->setPosition(
        m_arenaOffset + glm::vec3(bx, 2, -50 + bz));
    m_playerB->GetComponent<Transform>()->setRotation(
        glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
    m_playerB->GetComponent<Rigidbody>()->m_velocity = glm::vec3(0);
    m_playerB->GetComponent<Rigidbody>()->m_angularVelocity = glm::vec3(0);
    if (auto ecB = m_playerB->GetComponent<EnemyController>())
    {
        ecB->m_yaw = ecB->m_initialYaw;
        ecB->m_pitch = 0.0f;
    }

    glm::vec3 ballStart = m_arenaOffset + glm::vec3(ballX, 5, ballZ);
    m_ball->GetComponent<Transform>()->setPosition(ballStart);
    m_ball->GetComponent<Rigidbody>()->m_velocity =
        glm::vec3(velDist(gen), 0, velDist(gen));

    m_needsReset = false;
    m_framesSinceLastReset = 0;

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
    m_fitnessA += deltaDistToBallA * 0.5f;
    m_prevAgentAToBall = currDistToBallA;

    float currDistToBallB = glm::distance(agentBPos, ballPos);
    float deltaDistToBallB = m_prevAgentBToBall - currDistToBallB;
    m_fitnessB += deltaDistToBallB * 0.5f;
    m_prevAgentBToBall = currDistToBallB;

    float currBallToGateB = glm::distance(ballPos, m_gateBPos);
    float deltaBallA = m_prevBallToGateB - currBallToGateB;
    m_fitnessA += deltaBallA * 4.0f;
    m_prevBallToGateB = currBallToGateB;

    float currBallToGateA = glm::distance(ballPos, m_gateAPos);
    float deltaBallB = m_prevBallToGateA - currBallToGateA;
    m_fitnessB += deltaBallB * 4.0f;
    m_prevBallToGateA = currBallToGateA;

    m_fitnessA -= 0.05f;
    m_fitnessB -= 0.05f;
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

    if (m_autoReset && (m_needsReset || m_framesSinceLastReset > 1800))
    {
        resetPositions();
    }
}
