#include "enemyController.hpp"

#include <cassert>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "scripts/ai/math.hpp"
#include "scripts/ai/neuralAgent.hpp"
#include "scripts/footballer.hpp"
#include "world/components/rigidbody.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"

void EnemyController::onStart()
{
    Transform *transform = m_entity->GetComponent<Transform>();

    if (!transform)
        std::cerr << "Error: EnemyController requires transform component!\n";
}
void EnemyController::init(Entity *opponent, Entity *ball, glm::vec3 ownGatePos,
                           glm::vec3 enemyGatePos)
{
    m_opponent = opponent;
    m_ball = ball;
    m_ownGatePos = ownGatePos;
    m_enemyGatePos = enemyGatePos;
}

void EnemyController::onUpdate(float deltaTime)
{
    // ai agent inputs:
    // agent forward vector
    // agent velocity dir vector
    // agent velocity value
    // enemy relative position dir vector
    // enemy distance
    // enemy velocity dir
    // enemy velocity value
    // enemy forward vector
    // ball relative position dir vector
    // ball distance
    // ball velocity dir
    // vall velocity value
    // vector from agent to its own gate dir
    // vector from agent to its own gate value
    // vector from agent to opponent's gate dir
    // vector from agent to opponent's gate value
    // agents normalized speed parameter
    // agents normalized jump parameter
    // agents normalized kick parameter
    // is agent grounded
    Footballer *footballer = m_entity->GetComponent<Footballer>();
    NeuralAgent *agent = m_entity->GetComponent<NeuralAgent>();
    if (!footballer || !agent)
        return;

    static constexpr float MAX_SPEED = 30.0f;
    static constexpr float MAX_JUMP = 1550.0f;
    static constexpr float MAX_KICK = 5000.0f;
    static constexpr float BALL_CONST = 50.0f;
    static constexpr float AVG_SPEED = 20.0f;

    Transform *playerTrans = m_entity->GetComponent<Transform>();
    Transform *enemyTrans = m_opponent->GetComponent<Transform>();
    Transform *ballTrans = m_ball->GetComponent<Transform>();

    Rigidbody *playerRb = m_entity->GetComponent<Rigidbody>();
    Rigidbody *opponentRb = m_opponent->GetComponent<Rigidbody>();
    Rigidbody *ballRb = m_ball->GetComponent<Rigidbody>();

    uint16_t i = 0;
    const int NUM_INPUTS = 38; // Prawidłowa liczba wejść!
    Matrix agentAInput(NUM_INPUTS, 1);

    // 1. Player Forward
    glm::vec3 playerForward = playerTrans->getFront();
    agentAInput(i++, 0) = playerForward.x;
    agentAInput(i++, 0) = playerForward.y;
    agentAInput(i++, 0) = playerForward.z;

    // 2 & 3. Player Velocity Dir & Value
    glm::vec3 playerVel = playerRb->m_velocity;
    glm::vec3 playerDirVel = glm::vec3(0.0f);
    float playerSpeed = glm::length(playerVel);
    if (playerSpeed > 0.0001f)
        playerDirVel = playerVel / playerSpeed;

    agentAInput(i++, 0) = playerDirVel.x;
    agentAInput(i++, 0) = playerDirVel.y;
    agentAInput(i++, 0) = playerDirVel.z;
    agentAInput(i++, 0) = playerSpeed / (playerSpeed + AVG_SPEED);

    // 4 & 5. Enemy Position Dir & Distance (ZABEZPIECZONE!)
    glm::vec3 enemyRelPos =
        enemyTrans->getPosition() - playerTrans->getPosition();
    glm::vec3 enemyRelPosDir = glm::vec3(0.0f);
    float distToEnemy = glm::length(enemyRelPos);
    if (distToEnemy > 0.0001f)
        enemyRelPosDir = enemyRelPos / distToEnemy;

    agentAInput(i++, 0) = enemyRelPosDir.x;
    agentAInput(i++, 0) = enemyRelPosDir.y;
    agentAInput(i++, 0) = enemyRelPosDir.z;
    agentAInput(i++, 0) = distToEnemy / (1.0f + distToEnemy);

    // 6 & 7. Enemy Velocity Dir & Value
    glm::vec3 enemyVel = opponentRb->m_velocity;
    glm::vec3 enemyDirVel = glm::vec3(0.0f);
    float enemySpeed = glm::length(enemyVel);
    if (enemySpeed > 0.0001f)
        enemyDirVel = enemyVel / enemySpeed;

    agentAInput(i++, 0) = enemyDirVel.x;
    agentAInput(i++, 0) = enemyDirVel.y;
    agentAInput(i++, 0) = enemyDirVel.z;
    agentAInput(i++, 0) = enemySpeed / (enemySpeed + AVG_SPEED);

    // 8. Enemy Forward
    glm::vec3 enemyForward = enemyTrans->getFront();
    agentAInput(i++, 0) = enemyForward.x;
    agentAInput(i++, 0) = enemyForward.y;
    agentAInput(i++, 0) = enemyForward.z;

    // 9 & 10. Ball Relative Pos Dir & Distance
    glm::vec3 ballRelPos =
        ballTrans->getPosition() - playerTrans->getPosition();
    glm::vec3 ballRelPosDir = glm::vec3(0.0f);
    float distToBall = glm::length(ballRelPos);
    if (distToBall > 0.0001f)
        ballRelPosDir = ballRelPos / distToBall;

    agentAInput(i++, 0) = ballRelPosDir.x;
    agentAInput(i++, 0) = ballRelPosDir.y;
    agentAInput(i++, 0) = ballRelPosDir.z;
    agentAInput(i++, 0) = distToBall / (1.0f + distToBall);

    // 11 & 12. Ball Velocity Dir & Value
    glm::vec3 ballVel = ballRb->m_velocity;
    glm::vec3 ballDirVel = glm::vec3(0.0f);
    float ballSpeed = glm::length(ballVel);
    if (ballSpeed > 0.0001f)
        ballDirVel = ballVel / ballSpeed;

    agentAInput(i++, 0) = ballDirVel.x;
    agentAInput(i++, 0) = ballDirVel.y;
    agentAInput(i++, 0) = ballDirVel.z;
    agentAInput(i++, 0) = ballSpeed / (ballSpeed + BALL_CONST);

    // 13 & 14. Own Gate Relative Pos Dir & Distance
    glm::vec3 ownGateRelPos = m_ownGatePos - playerTrans->getPosition();
    glm::vec3 ownGateRelPosDir = glm::vec3(0.0f);
    float distToOwnGate = glm::length(ownGateRelPos);
    if (distToOwnGate > 0.0001f)
        ownGateRelPosDir = ownGateRelPos / distToOwnGate;

    agentAInput(i++, 0) = ownGateRelPosDir.x;
    agentAInput(i++, 0) = ownGateRelPosDir.y;
    agentAInput(i++, 0) = ownGateRelPosDir.z;
    agentAInput(i++, 0) = distToOwnGate / (1.0f + distToOwnGate);

    // 15 & 16. Enemy Gate Relative Pos Dir & Distance
    glm::vec3 enemyGateRelPos = m_enemyGatePos - playerTrans->getPosition();
    glm::vec3 enemyGateRelPosDir = glm::vec3(0.0f);
    float distToEnemyGate = glm::length(enemyGateRelPos);
    if (distToEnemyGate > 0.0001f)
        enemyGateRelPosDir = enemyGateRelPos / distToEnemyGate;

    agentAInput(i++, 0) = enemyGateRelPosDir.x;
    agentAInput(i++, 0) = enemyGateRelPosDir.y;
    agentAInput(i++, 0) = enemyGateRelPosDir.z;
    agentAInput(i++, 0) = distToEnemyGate / (1.0f + distToEnemyGate);

    // 17, 18, 19, 20. Parameters & Grounded
    float normSpeed = footballer->m_speed / MAX_SPEED;
    float normJump = footballer->m_jumpHeight / MAX_JUMP;
    float normKick = footballer->m_kickStrength / MAX_KICK;
    bool isGrounded = footballer->m_groundTimer > 0;

    agentAInput(i++, 0) = normSpeed;
    agentAInput(i++, 0) = normJump;
    agentAInput(i++, 0) = normKick;
    agentAInput(i++, 0) = (isGrounded ? 1.0f : 0.0f);

    assert(i == NUM_INPUTS &&
           "Number of generated input is different from size of the matrix!");

    Matrix outputs = agent->predict(agentAInput);

    // Forward / Backward
    float moveY = (outputs(0, 0) * 2.0f) - 1.0f;
    // Deadzone
    if (std::abs(moveY) < 0.2f)
        moveY = 0.0f;
    footballer->m_input.y = moveY;

    // Left / Right
    float moveX = (outputs(1, 0) * 2.0f) - 1.0f;
    if (std::abs(moveX) < 0.2f)
        moveX = 0.0f;
    footballer->m_input.x = moveX;

    // Jump
    if (outputs(2, 0) > 0.5f)
    {
        footballer->m_jump = true;
    }

    // Kick
    if (outputs(3, 0) > 0.5f)
    {
        if (onKickReward)
        {
            glm::vec3 toEnemyGate =
                glm::normalize(m_enemyGatePos - ballTrans->getPosition());
            footballer->kickBall();
            glm::vec3 kickDir = glm::normalize(ballRb->m_velocity);
            float alignment = glm::dot(kickDir, toEnemyGate);
            if (alignment > 0.0f)
                onKickReward(alignment * 100.0f);
        }
        else
        {
            footballer->kickBall();
        }
    }
}
