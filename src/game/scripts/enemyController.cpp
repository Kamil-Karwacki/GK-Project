#include "enemyController.hpp"
#include "scripts/ai/neuralAgent.hpp"
#include "scripts/footballer.hpp"
#include "world/components/rigidbody.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"
#include <cassert>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
    Footballer *footballer = m_entity->GetComponent<Footballer>();
    NeuralAgent *agent = m_entity->GetComponent<NeuralAgent>();
    if (!footballer || !agent)
        return;

    m_frameCounter++;

    if (m_frameCounter % m_frameSkip == 0)
    {
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
        const int NUM_INPUTS = 40;

        glm::vec3 forward = playerTrans->getFront();
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(up, forward));

        m_inputMatrix(i++, 0) = forward.x;
        m_inputMatrix(i++, 0) = forward.y;
        m_inputMatrix(i++, 0) = forward.z;

        glm::vec3 playerVel = playerRb->m_velocity;
        glm::vec3 playerDirVel = glm::vec3(0.0f);
        float playerSpeed = glm::length(playerVel);
        if (playerSpeed > 0.0001f)
            playerDirVel = playerVel / playerSpeed;

        m_inputMatrix(i++, 0) = glm::dot(playerDirVel, right);
        m_inputMatrix(i++, 0) = glm::dot(playerDirVel, up);
        m_inputMatrix(i++, 0) = glm::dot(playerDirVel, forward);
        m_inputMatrix(i++, 0) = playerSpeed / (playerSpeed + AVG_SPEED);

        glm::vec3 enemyRelPos =
            enemyTrans->getPosition() - playerTrans->getPosition();
        glm::vec3 enemyRelPosDir = glm::vec3(0.0f);
        float distToEnemy = glm::length(enemyRelPos);
        if (distToEnemy > 0.0001f)
            enemyRelPosDir = enemyRelPos / distToEnemy;

        m_inputMatrix(i++, 0) = glm::dot(enemyRelPosDir, right);
        m_inputMatrix(i++, 0) = glm::dot(enemyRelPosDir, up);
        m_inputMatrix(i++, 0) = glm::dot(enemyRelPosDir, forward);
        m_inputMatrix(i++, 0) = distToEnemy / (1.0f + distToEnemy);

        glm::vec3 enemyVel = opponentRb->m_velocity;
        glm::vec3 enemyDirVel = glm::vec3(0.0f);
        float enemySpeed = glm::length(enemyVel);
        if (enemySpeed > 0.0001f)
            enemyDirVel = enemyVel / enemySpeed;

        m_inputMatrix(i++, 0) = glm::dot(enemyDirVel, right);
        m_inputMatrix(i++, 0) = glm::dot(enemyDirVel, up);
        m_inputMatrix(i++, 0) = glm::dot(enemyDirVel, forward);
        m_inputMatrix(i++, 0) = enemySpeed / (enemySpeed + AVG_SPEED);

        glm::vec3 enemyForward = enemyTrans->getFront();
        m_inputMatrix(i++, 0) = glm::dot(enemyForward, right);
        m_inputMatrix(i++, 0) = glm::dot(enemyForward, up);
        m_inputMatrix(i++, 0) = glm::dot(enemyForward, forward);

        glm::vec3 ballRelPos =
            ballTrans->getPosition() - playerTrans->getPosition();
        glm::vec3 ballRelPosDir = glm::vec3(0.0f);
        float distToBall = glm::length(ballRelPos);
        if (distToBall > 0.0001f)
            ballRelPosDir = ballRelPos / distToBall;

        m_inputMatrix(i++, 0) = glm::dot(ballRelPosDir, right);
        m_inputMatrix(i++, 0) = glm::dot(ballRelPosDir, up);
        m_inputMatrix(i++, 0) = glm::dot(ballRelPosDir, forward);
        m_inputMatrix(i++, 0) = distToBall / (1.0f + distToBall);

        glm::vec3 ballVel = ballRb->m_velocity;
        glm::vec3 ballDirVel = glm::vec3(0.0f);
        float ballSpeed = glm::length(ballVel);
        if (ballSpeed > 0.0001f)
            ballDirVel = ballVel / ballSpeed;

        m_inputMatrix(i++, 0) = glm::dot(ballDirVel, right);
        m_inputMatrix(i++, 0) = glm::dot(ballDirVel, up);
        m_inputMatrix(i++, 0) = glm::dot(ballDirVel, forward);
        m_inputMatrix(i++, 0) = ballSpeed / (ballSpeed + BALL_CONST);

        glm::vec3 ownGateRelPos = m_ownGatePos - playerTrans->getPosition();
        glm::vec3 ownGateRelPosDir = glm::vec3(0.0f);
        float distToOwnGate = glm::length(ownGateRelPos);
        if (distToOwnGate > 0.0001f)
            ownGateRelPosDir = ownGateRelPos / distToOwnGate;

        m_inputMatrix(i++, 0) = glm::dot(ownGateRelPosDir, right);
        m_inputMatrix(i++, 0) = glm::dot(ownGateRelPosDir, up);
        m_inputMatrix(i++, 0) = glm::dot(ownGateRelPosDir, forward);
        m_inputMatrix(i++, 0) = distToOwnGate / (1.0f + distToOwnGate);

        glm::vec3 enemyGateRelPos = m_enemyGatePos - playerTrans->getPosition();
        glm::vec3 enemyGateRelPosDir = glm::vec3(0.0f);
        float distToEnemyGate = glm::length(enemyGateRelPos);
        if (distToEnemyGate > 0.0001f)
            enemyGateRelPosDir = enemyGateRelPos / distToEnemyGate;

        m_inputMatrix(i++, 0) = glm::dot(enemyGateRelPosDir, right);
        m_inputMatrix(i++, 0) = glm::dot(enemyGateRelPosDir, up);
        m_inputMatrix(i++, 0) = glm::dot(enemyGateRelPosDir, forward);
        m_inputMatrix(i++, 0) = distToEnemyGate / (1.0f + distToEnemyGate);

        float normSpeed = footballer->m_speed / MAX_SPEED;
        float normJump = footballer->m_jumpHeight / MAX_JUMP;
        float normKick = footballer->m_kickStrength / MAX_KICK;
        bool isGrounded = footballer->m_groundTimer > 0;

        m_inputMatrix(i++, 0) = normSpeed;
        m_inputMatrix(i++, 0) = normJump;
        m_inputMatrix(i++, 0) = normKick;
        m_inputMatrix(i++, 0) = (isGrounded ? 1.0f : 0.0f);

        glm::vec3 predictedBallPos =
            ballTrans->getPosition() + ballRb->m_velocity * 0.3f;
        glm::vec3 predictedBallRel =
            predictedBallPos - playerTrans->getPosition();
        glm::vec3 predictedBallDir = glm::vec3(0.0f);
        float predictedBallDist = glm::length(predictedBallRel);
        if (predictedBallDist > 0.0001f)
            predictedBallDir = predictedBallRel / predictedBallDist;

        m_inputMatrix(i++, 0) = glm::dot(predictedBallDir, right);
        m_inputMatrix(i++, 0) = glm::dot(predictedBallDir, forward);

        Matrix outputs = agent->predict(m_inputMatrix);
        m_lastMoveY = (outputs(0, 0) * 2.0f) - 1.0f;
        m_lastMoveX = (outputs(1, 0) * 2.0f) - 1.0f;
        m_lastJump = outputs(2, 0) > 0.5f;
        m_lastKick = outputs(3, 0) > 0.5f;
        m_lastTurnYaw = (outputs(4, 0) * 2.0f) - 1.0f;
        m_lastTurnPitch = (outputs(5, 0) * 2.0f) - 1.0f;

        assert(
            i == NUM_INPUTS &&
            "Number of generated input is different from size of the matrix!");
    }

    footballer->m_input.y = m_lastMoveY;
    footballer->m_input.x = m_lastMoveX;

    if (m_lastJump)
        footballer->m_jump = true;

    if (m_lastKick)
    {
        if (onKickReward)
        {
            glm::vec3 toEnemyGate = glm::normalize(
                m_enemyGatePos -
                m_ball->GetComponent<Transform>()->getPosition());
            footballer->kickBall();

            glm::vec3 ballVel = m_ball->GetComponent<Rigidbody>()->m_velocity;
            glm::vec3 kickDir = glm::vec3(0.0f);
            if (glm::length(ballVel) > 0.0001f)
            {
                kickDir = glm::normalize(ballVel);
            }
            float alignment = glm::dot(kickDir, toEnemyGate);

            if (alignment > 0.0f)
                onKickReward(alignment * 300.0f);
        }
        else
        {
            footballer->kickBall();
        }
        m_lastKick = false;
    }

    float yawRange = glm::radians(180.0f);
    float pitchRange = glm::radians(25.0f);

    m_yaw = -m_lastTurnYaw * yawRange;
    m_pitch = -m_lastTurnPitch * pitchRange;

    footballer->m_rotation = glm::vec2(m_pitch, m_yaw);
}
