
#include "headlessTrainerScene.hpp"

#include <cstdint>
#include <memory>

#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
#include "scripts/ai/neuralAgent.hpp"
#include "scripts/ball.hpp"
#include "scripts/enemyController.hpp"
#include "scripts/footballer.hpp"
#include "scripts/footballerShootTrigger.hpp"
#include "scripts/gateTrigger.hpp"
#include "scripts/playerGrounded.hpp"
#include "world/components/collider.hpp"
#include "world/components/rigidbody.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"
#include "world/scene.hpp"

void HeadlessTrainerScene::init()
{
    for (int i = 0; i < 500; i++)
    {
        MatchArena arena(i);
        generateArena(arena);
        m_arenas.push_back(std::move(arena));
    }

    std::cout << "Headless trainer scene initialized successfully\n";
}
void HeadlessTrainerScene::generateArena(MatchArena &arena)
{
    glm::vec2 pitchSize = glm::vec2(115, 74);
    pitchSize *= 1.4f;
    float wallHeight = 4.0f;
    float bannerLength = 32.0f;

    glm::vec2 tribuneOffset = glm::vec2(0, 10);
    glm::vec2 groundAdd = glm::vec2(tribuneOffset.y * 2);
    generatePitch(arena, pitchSize, groundAdd, wallHeight, bannerLength);
    glm::vec3 gateSize = glm::vec3(30.0f, 11.0f, 11.0f);
    float gateThickness = 0.7f;

    generateGates(arena, pitchSize, gateSize, gateThickness);

    // player A
    Entity &player = arena.createEntity(this);
    player.AddComponent<Transform>(arena.m_arenaOffset + glm::vec3(0, 10, 50),
                                   glm::vec3(0), glm::vec3(1.5f));

    EnemyController &enemyA = player.AddComponent<EnemyController>();
    player.AddComponent<Rigidbody>(10.0f, 0.1f, 0.5f, 0.99f, 0.99f);
    SphereCollider &playerCol =
        player.AddComponentAs<Collider, SphereCollider>(1.5f);
    playerCol.m_restitution = 0.0f;
    playerCol.m_layer = CAT_PLAYER;
    playerCol.m_mask = CAT_BALL | CAT_ENEMY | CAT_GROUND;
    player.GetComponent<Rigidbody>()->m_invInertiaTensor =
        Rigidbody::createSphereInverseInertiaTensor(1.0f, 2.0f);
    player.AddComponent<Footballer>();
    NeuralAgent &neuralA = player.AddComponent<NeuralAgent>(38, 64, 6);

    Entity &playerShootTrigger = arena.createEntity(this);
    playerShootTrigger.AddComponent<Transform>();
    playerShootTrigger.AddComponent<FootballerShootTrigger>(&player);
    glm::mat4 offset = glm::mat4(1.0f);
    offset = glm::translate(offset, glm::vec3(0, 0, 3.6f));
    SphereCollider &playerBallCol =
        playerShootTrigger.AddComponentAs<Collider, SphereCollider>(
            1.1f, offset, true);
    playerBallCol.m_layer = CAT_PLAYER;
    playerBallCol.m_mask = CAT_BALL;

    Entity &playerGrounded = arena.createEntity(this);
    playerGrounded.AddComponent<Transform>();
    SphereCollider &groundCol =
        playerGrounded.AddComponentAs<Collider, SphereCollider>(
            0.5f, glm::mat4(1.0f), true);
    groundCol.m_layer = CAT_PLAYER;
    groundCol.m_mask = CAT_GROUND | CAT_BALL | CAT_ENEMY;
    playerGrounded.AddComponent<PlayerGrounded>(&player,
                                                glm::vec3(0, -1.1f, 0));

    // player B
    Entity &enemy = arena.createEntity(this);
    enemy.AddComponent<Transform>(arena.m_arenaOffset + glm::vec3(0, 10, -50),
                                  glm::vec3(0), glm::vec3(1.5f));

    EnemyController &enemyB = enemy.AddComponent<EnemyController>();
    enemy.AddComponent<Rigidbody>(10.0f, 0.1f, 0.5f, 0.99f, 0.99f);
    SphereCollider &enemyCol =
        enemy.AddComponentAs<Collider, SphereCollider>(1.5f);
    enemyCol.m_restitution = 0.0f;
    enemyCol.m_layer = CAT_ENEMY;
    enemyCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_GROUND;
    enemy.GetComponent<Rigidbody>()->m_invInertiaTensor =
        Rigidbody::createSphereInverseInertiaTensor(1.0f, 2.0f);
    enemy.AddComponent<Footballer>();
    NeuralAgent &neuralB = enemy.AddComponent<NeuralAgent>(38, 64, 6);

    Entity &enemyShootTrigger = arena.createEntity(this);
    enemyShootTrigger.AddComponent<Transform>();
    enemyShootTrigger.AddComponent<FootballerShootTrigger>(&enemy);
    offset = glm::mat4(1.0f);
    offset = glm::translate(offset, glm::vec3(0, 0, 3.6f));
    SphereCollider &enemyBallCol =
        enemyShootTrigger.AddComponentAs<Collider, SphereCollider>(
            1.1f, offset, true); // Poprawka nazwy
    enemyBallCol.m_layer = CAT_PLAYER;
    enemyBallCol.m_mask = CAT_BALL;

    Entity &enemyGrounded = arena.createEntity(this);
    enemyGrounded.AddComponent<Transform>();
    SphereCollider &enemyGroundCol =
        enemyGrounded.AddComponentAs<Collider, SphereCollider>(
            0.5f, glm::mat4(1.0f), true);
    enemyGroundCol.m_layer = CAT_ENEMY;
    enemyGroundCol.m_mask = CAT_GROUND | CAT_BALL | CAT_PLAYER;
    enemyGrounded.AddComponent<PlayerGrounded>(&enemy, glm::vec3(0, -1.1f, 0));

    // ball
    Entity &sphere = arena.createEntity(this);
    sphere.AddComponent<Transform>();
    sphere.GetComponent<Transform>()->setScale(glm::vec3(2.5f));
    sphere.GetComponent<Transform>()->setPosition(arena.m_arenaOffset +
                                                  glm::vec3(0.0f, 5.0f, 0.0f));

    SphereCollider &ballCol =
        sphere.AddComponentAs<Collider, SphereCollider>(2.0f);
    ballCol.m_restitution = 0.6f;
    ballCol.m_friction = 1.0f;
    ballCol.m_layer = CAT_BALL;
    ballCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_ENEMY | CAT_GROUND;

    sphere.AddComponent<Rigidbody>(0.5f, 0.3f, 30.0f, 0.8f, 0.8f);
    Rigidbody *sphereRb = sphere.GetComponent<Rigidbody>();
    sphereRb->m_invInertiaTensor =
        Rigidbody::createSphereInverseInertiaTensor(10.0f, 2.0f);
    sphere.AddComponent<Ball>();

    arena.m_playerA = &player;
    arena.m_playerB = &enemy;
    arena.m_ball = &sphere;

    enemyA.init(&enemy, &sphere, arena.m_gateAPos, arena.m_gateBPos);
    enemyB.init(&player, &sphere, arena.m_gateBPos, arena.m_gateAPos);

    enemyA.onKickReward = [&arena](float reward)
    { arena.m_fitnessA += reward; };
    enemyB.onKickReward = [&arena](float reward)
    { arena.m_fitnessB += reward; };
}

void HeadlessTrainerScene::update(float deltaTime) { Scene::update(deltaTime); }

void HeadlessTrainerScene::fixedUpdate(float deltaTime)
{
    static constexpr float gravity = 42.0f;

#pragma omp parallel for
    for (auto &arena : m_arenas)
    {
        for (auto &entity : arena.m_entities)
        {
            Rigidbody *rb = entity->GetComponent<Rigidbody>();
            if (rb)
            {
                rb->m_forceAcc += glm::vec3(0.0f, -1.0f, 0.0f) * gravity *
                                  (1.0f / rb->m_inverseMass);
            }
        }
        arena.fixedUpdate(deltaTime);
        arena.m_framesSinceLastReset++;

        Transform *transA = arena.m_playerA->GetComponent<Transform>();
        Transform *transB = arena.m_playerB->GetComponent<Transform>();
        Transform *ballTrans = arena.m_ball->GetComponent<Transform>();

        Rigidbody *ballRb = arena.m_ball->GetComponent<Rigidbody>();

        EnemyController *enemyA =
            arena.m_playerA->GetComponent<EnemyController>();
        EnemyController *enemyB =
            arena.m_playerB->GetComponent<EnemyController>();

        glm::vec3 ballPos = ballTrans->getPosition();
        glm::vec3 agentAPos = transA->getPosition();
        glm::vec3 agentBPos = transB->getPosition();

        float distABall = glm::distance(agentAPos, ballPos);
        float distBBall = glm::distance(agentBPos, ballPos);

        // Reward for pushing the ball closer to enemys gate
        float currBallToGateB = glm::distance(ballPos, arena.m_gateBPos);
        float currBallToGateA = glm::distance(ballPos, arena.m_gateAPos);

        float deltaA = arena.m_prevBallToGateB - currBallToGateB;
        float deltaB = arena.m_prevBallToGateA - currBallToGateA;

        if (deltaA > 0.0f)
            arena.m_fitnessA += deltaA * 0.5f;
        if (deltaB > 0.0f)
            arena.m_fitnessB += deltaB * 0.5f;

        arena.m_prevBallToGateB = currBallToGateB;
        arena.m_prevBallToGateA = currBallToGateA;

        // Reward for being behind the ball in line with the gaet
        /*distABall = glm::distance(agentAPos, ballPos);
        distBBall = glm::distance(agentBPos, ballPos);

        if (distABall > 0.5f && currBallToGateB > 0.5f)
        {
            glm::vec3 ballToGateB = glm::normalize(arena.m_gateBPos - ballPos);
            glm::vec3 ballToAgentA = glm::normalize(agentAPos - ballPos);
            float positioningA = glm::dot(ballToAgentA, ballToGateB); // -1..1
            if (positioningA > 0.0f)
                arena.m_fitnessA += positioningA * 0.015f;
        }

        if (distBBall > 0.5f && currBallToGateA > 0.5f)
        {
            glm::vec3 ballToGateA = glm::normalize(arena.m_gateAPos - ballPos);
            glm::vec3 ballToAgentB = glm::normalize(agentBPos - ballPos);
            float positioningB = glm::dot(ballToAgentB, ballToGateA); // -1..1
            if (positioningB > 0.0f)
                arena.m_fitnessB += positioningB * 0.015f;
        }*/

        // Punishment for being too far from the ball
        if (distABall > 70.0f)
            arena.m_fitnessA -= 0.01f;
        if (distBBall > 70.0f)
            arena.m_fitnessB -= 0.01f;

        if (arena.m_needsReset || arena.m_framesSinceLastReset > 1800)
        {
            arena.resetPositions();
        }
    }
}

void HeadlessTrainerScene::generatePitch(MatchArena &arena, glm::vec2 pitchSize,
                                         glm::vec2 groundAdd, float wallHeight,
                                         float bannerLength)
{
    Entity &ground = arena.createEntity(this);
    ground.AddComponent<Transform>(arena.m_arenaOffset);

    Collider &groundCol = ground.AddComponentAs<Collider, HalfspaceCollider>(
        glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);
    groundCol.m_layer = CAT_GROUND;
    groundCol.m_mask = CAT_PLAYER | CAT_ENEMY | CAT_BALL;
    groundCol.m_restitution = 0.4f;

    glm::vec2 bannerCount = glm::round(pitchSize / bannerLength);
    glm::vec2 bannerLengths = pitchSize / bannerCount;
    glm::vec2 bannerScale = bannerLengths / bannerLength;

    for (size_t i = 0; i < bannerCount.y; i++)
    {
        Entity &wallA = arena.createEntity(this);
        Transform &transA = wallA.AddComponent<Transform>();
        transA.setPosition(arena.m_arenaOffset +
                           glm::vec3((pitchSize.y / 2.0f) -
                                         (bannerLengths.y / 2.0f) -
                                         bannerLengths.y * i,
                                     0.0f, pitchSize.x / 2.0f));
        transA.setScale(glm::vec3(bannerScale.y, 1.0f, 1.0f));
        transA.setRotation(
            glm::vec3(glm::radians(90.0f), glm::radians(180.0f), 0));

        if (i == 0)
        {
            glm::vec3 normal(0.0f, 0.0f, -1.0f);
            float d =
                (-pitchSize.x / 2.0f) + glm::dot(normal, arena.m_arenaOffset);
            auto &wallCol =
                wallA.AddComponentAs<Collider, HalfspaceCollider>(normal, d);
            wallCol.m_layer = CAT_GROUND;
            wallCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_ENEMY;
        }

        Entity &wallB = arena.createEntity(this);
        Transform &transB = wallB.AddComponent<Transform>();
        transB.setPosition(arena.m_arenaOffset +
                           glm::vec3((pitchSize.y / 2.0f) -
                                         (bannerLengths.y / 2.0f) -
                                         bannerLengths.y * i,
                                     0.0f, -pitchSize.x / 2.0f));
        transB.setScale(glm::vec3(bannerScale.y, 1.0f, 1.0f));
        transB.setRotation(
            glm::vec3(glm::radians(90.0f), glm::radians(180.0f), 0));
        if (i == 0)
        {
            glm::vec3 normal(0.0f, 0.0f, 1.0f);
            float d =
                (-pitchSize.x / 2.0f) + glm::dot(normal, arena.m_arenaOffset);
            HalfspaceCollider &wallCol =
                wallB.AddComponentAs<Collider, HalfspaceCollider>(normal, d);
            wallCol.m_layer = CAT_GROUND;
            wallCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_ENEMY;
        }
    }

    for (size_t i = 0; i < bannerCount.x; i++)
    {
        Entity &wallA = arena.createEntity(this);
        Transform &transA = wallA.AddComponent<Transform>();
        transA.setPosition(arena.m_arenaOffset +
                           glm::vec3(pitchSize.y / 2.0f, 0.0f,
                                     (-pitchSize.x / 2.0f) +
                                         (bannerLengths.x / 2.0f) +
                                         bannerLengths.x * i));
        transA.setScale(glm::vec3(bannerScale.x, 1.0f, 1.0f));
        transA.setRotation(
            glm::vec3(glm::radians(90.0f), glm::radians(270.0f), 0));

        if (i == 0)
        {
            glm::vec3 normal(-1.0f, 0.0f, 0.0f);
            float d =
                (-pitchSize.y / 2.0f) + glm::dot(normal, arena.m_arenaOffset);
            HalfspaceCollider &wallCol =
                wallA.AddComponentAs<Collider, HalfspaceCollider>(normal, d);
            wallCol.m_layer = CAT_GROUND;
            wallCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_ENEMY;
        }

        Entity &wallB = arena.createEntity(this);
        Transform &transB = wallB.AddComponent<Transform>();
        transB.setPosition(arena.m_arenaOffset +
                           glm::vec3(-pitchSize.y / 2.0f, 0.0f,
                                     (-pitchSize.x / 2.0f) +
                                         (bannerLengths.x / 2.0f) +
                                         bannerLengths.x * i));
        transB.setScale(glm::vec3(bannerScale.x, 1.0f, 1.0f));
        transB.setRotation(
            glm::vec3(glm::radians(90.0f), glm::radians(90.0f), 0));

        if (i == 0)
        {
            glm::vec3 normal(1.0f, 0.0f, 0.0f);
            float d =
                (-pitchSize.y / 2.0f) + glm::dot(normal, arena.m_arenaOffset);
            HalfspaceCollider &wallCol =
                wallB.AddComponentAs<Collider, HalfspaceCollider>(normal, d);
            wallCol.m_layer = CAT_GROUND;
            wallCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_ENEMY;
        }
    }
}

void HeadlessTrainerScene::generateGates(MatchArena &arena, glm::vec2 pitchSize,
                                         glm::vec3 gateSize,
                                         float gateThickness)
{
    for (int j = 1; j > -2; j -= 2)
    {
        for (int i = 1; i > -2; i -= 2)
        {
            Entity &bar = arena.createEntity(this);
            Transform &trans = bar.AddComponent<Transform>();
            trans.setPosition(
                arena.m_arenaOffset +
                glm::vec3(i * gateSize.x / 2.0f, gateSize.y / 2.0f,
                          j * pitchSize.x / 2.0f - gateSize.z * j));

            BoxCollider &boxCol = bar.AddComponentAs<Collider, BoxCollider>(
                glm::vec3(gateThickness / 2.0f, gateSize.y / 2.0f,
                          gateSize.z / 2.0f));
            boxCol.m_offset = glm::translate(
                boxCol.m_offset, glm::vec3(0, 0, j * gateSize.z / 2));
            boxCol.m_layer = CAT_GROUND;
            boxCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_ENEMY;
        }
    }

    float ballRadius = 2.5f;
    uint32_t currentId = arena.m_arenaID;

    Entity &goalTriggerA = arena.createEntity(this);
    goalTriggerA.AddComponent<Transform>(
        arena.m_arenaOffset +
        glm::vec3{0.0f, gateSize.y / 2.0f,
                  pitchSize.x / 2.0f - gateSize.z / 2.0f + ballRadius * 2});
    goalTriggerA.AddComponentAs<Collider, BoxCollider>(
        glm::vec3{gateSize.x * 0.9f / 2.0f, gateSize.y / 2.0f,
                  gateSize.z / 2.0f},
        glm::mat4(1.0f), true);
    goalTriggerA.AddComponent<GateTrigger>(
        [this, currentId]()
        {
            this->m_arenas[currentId].m_fitnessB += 1000.0f;
            this->m_arenas[currentId].m_fitnessA -= 500.0f;
            this->m_arenas[currentId].m_needsReset = true;
        });

    Entity &goalTriggerB = arena.createEntity(this);
    goalTriggerB.AddComponent<Transform>(
        arena.m_arenaOffset +
        glm::vec3{0.0f, gateSize.y / 2.0f,
                  -pitchSize.x / 2.0f + gateSize.z / 2.0f - ballRadius * 2});
    goalTriggerB.AddComponentAs<Collider, BoxCollider>(
        glm::vec3{gateSize.x * 0.9f / 2.0f, gateSize.y / 2.0f,
                  gateSize.z / 2.0f},
        glm::mat4(1.0f), true);
    goalTriggerB.AddComponent<GateTrigger>(
        [this, currentId]()
        {
            this->m_arenas[currentId].m_fitnessA += 1000.0f;
            this->m_arenas[currentId].m_fitnessB -= 500.0f;
            this->m_arenas[currentId].m_needsReset = true;
        });

    for (int i = 1; i > -2; i -= 2)
    {
        Entity &topBar = arena.createEntity(this);
        Transform &topTrans = topBar.AddComponent<Transform>();
        topTrans.setPosition(
            arena.m_arenaOffset +
            glm::vec3(0.0f, gateSize.y,
                      i * pitchSize.x / 2.0f - gateSize.z * i));
        topTrans.setRotation(glm::vec3(0.0f, 0.0f, glm::radians(90.0f)));
        BoxCollider &boxCol =
            topBar.AddComponentAs<Collider, BoxCollider>(glm::vec3(
                gateThickness / 2.0f, gateSize.x / 2.0f, gateSize.z / 2.0f));
        boxCol.m_offset = glm::translate(boxCol.m_offset,
                                         glm::vec3(0, 0, i * gateSize.z / 2));
        boxCol.m_layer = CAT_GROUND;
        boxCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_ENEMY;
    }

    arena.m_gateAPos =
        arena.m_arenaOffset +
        glm::vec3(0.0f, gateSize.y / 2.0f, pitchSize.x / 2.0f - gateSize.z);
    arena.m_gateBPos =
        arena.m_arenaOffset +
        glm::vec3(0.0f, gateSize.y / 2.0f, -pitchSize.x / 2.0f + gateSize.z);
}
