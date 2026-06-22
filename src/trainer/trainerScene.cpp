
#include "trainerScene.hpp"

#include <cstdint>
#include <memory>

#include "core/application.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
#include "graphics/model.hpp"
#include "graphics/renderSystem.hpp"
#include "scripts/ai/neuralAgent.hpp"
#include "scripts/ball.hpp"
#include "scripts/enemyController.hpp"
#include "scripts/footballer.hpp"
#include "scripts/footballerShootTrigger.hpp"
#include "scripts/freeCameraController.hpp"
#include "scripts/gateTrigger.hpp"
#include "scripts/pitchGenerator.hpp"
#include "scripts/playerGrounded.hpp"
#include "world/baseScene.hpp"
#include "world/components/collider.hpp"
#include "world/components/light.hpp"
#include "world/components/meshRenderer.hpp"
#include "world/components/rigidbody.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"
#include "world/scene.hpp"

void TrainerScene::init()
{
    for (int i = 0; i < 300; i++)
    {
        MatchArena arena(i);
        generateArena(arena);
        m_arenas.push_back(std::move(arena));
    }
    Entity &sun = createEntity();
    sun.AddComponent<Transform>(glm::vec3(0, 50, 0),
                                glm::vec3(glm::radians(290.0f), 0, 0));
    sun.AddComponent<DirectionalLight>(glm::vec3(1.0f, 1.0f, 1.0f), 0.3f, 0.6f,
                                       0.4f);

    Entity &cameraPlayer = createEntity();
    Transform &cameraTrans = cameraPlayer.AddComponent<Transform>();
    m_mainCamera = &cameraPlayer.AddComponent<Camera>();
    cameraTrans.setPosition({150, 450, 10});
    cameraTrans.setRotation(glm::vec3(glm::radians(-90.0f), 0, 0));
    cameraPlayer.AddComponent<FreeCameraController>();
    std::cout << "Trainer scene initialized successfully\n";
}
void TrainerScene::generateArena(MatchArena &arena)
{
    Application &app = Application::Get();
    Shader *defaultShader = app.getShader("default");

    PitchGenerator::PitchConfig config;
    config.pitchSize = glm::vec2(115, 74) * 1.4f;
    config.wallHeight = 4.0f;
    config.bannerLength = 32.0f;
    config.gateSize = glm::vec3(30.0f, 11.0f, 11.0f);
    config.gateThickness = 0.7f;

    PitchGenerator::generatePitch(arena.m_entities, this, arena.m_arenaOffset,
                                  defaultShader, config);

    uint32_t currentId = arena.m_arenaID;

    auto onGoalA = [this, currentId]() { this->m_arenas[currentId].onGoalA(); };
    auto onGoalB = [this, currentId]() { this->m_arenas[currentId].onGoalB(); };

    auto gatesInfo = PitchGenerator::generateGates(
        arena.m_entities, this, arena.m_arenaOffset, defaultShader, onGoalA,
        onGoalB, config);

    arena.m_gateAPos = gatesInfo.gateAPos;
    arena.m_gateBPos = gatesInfo.gateBPos;

    std::shared_ptr<Model> playerModel =
        std::make_shared<Model>("assets/models/redBall.obj");
    std::shared_ptr<Model> ballModel =
        std::make_shared<Model>("assets/models/ball.obj");
    std::shared_ptr<Model> redBallModel =
        std::make_shared<Model>("assets/models/redBall.obj");

    // player A
    Entity &player = arena.createEntity(this);
    player.AddComponent<Transform>(arena.m_arenaOffset + glm::vec3(0, 10, 50),
                                   glm::vec3(0, glm::radians(180.0f), 0),
                                   glm::vec3(1.5f));

    EnemyController &enemyA = player.AddComponent<EnemyController>();
    player.AddComponent<MeshRenderer>(playerModel, defaultShader);
    player.AddComponent<Rigidbody>(10.0f, 0.1f, 0.5f, 0.99f, 0.99f);
    SphereCollider &playerCol =
        player.AddComponentAs<Collider, SphereCollider>(1.5f);
    playerCol.m_restitution = 0.0f;
    playerCol.m_layer = CAT_PLAYER;
    playerCol.m_mask = CAT_BALL | CAT_ENEMY | CAT_GROUND;
    player.GetComponent<Rigidbody>()->m_invInertiaTensor =
        Rigidbody::createSphereInverseInertiaTensor(1.0f, 2.0f);
    player.AddComponent<Footballer>();
    NeuralAgent &neuralA = player.AddComponent<NeuralAgent>(43, 256, 6);
    neuralA.loadFromFile("best_brain.txt");

    Entity &playerShootTrigger = arena.createEntity(this);
    playerShootTrigger.AddComponent<Transform>();
    playerShootTrigger.AddComponent<FootballerShootTrigger>(&player);
    glm::mat4 offset = glm::mat4(1.0f);
    offset = glm::translate(offset, glm::vec3(0, 0, -3.6f));
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
                                  glm::vec3(0, glm::radians(180.0f), 0),
                                  glm::vec3(1.5f));

    EnemyController &enemyB = enemy.AddComponent<EnemyController>();
    enemy.AddComponent<MeshRenderer>(redBallModel, defaultShader);
    enemy.AddComponent<Rigidbody>(10.0f, 0.1f, 0.5f, 0.99f, 0.99f);
    SphereCollider &enemyCol =
        enemy.AddComponentAs<Collider, SphereCollider>(1.5f);
    enemyCol.m_restitution = 0.0f;
    enemyCol.m_layer = CAT_ENEMY;
    enemyCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_GROUND;
    enemy.GetComponent<Rigidbody>()->m_invInertiaTensor =
        Rigidbody::createSphereInverseInertiaTensor(1.0f, 2.0f);
    enemy.AddComponent<Footballer>();
    NeuralAgent &neuralB = enemy.AddComponent<NeuralAgent>(43, 256, 6);
    neuralB.loadFromFile("best_brain.txt");

    Entity &enemyShootTrigger = arena.createEntity(this);
    enemyShootTrigger.AddComponent<Transform>();
    enemyShootTrigger.AddComponent<FootballerShootTrigger>(&enemy);
    offset = glm::mat4(1.0f);
    offset = glm::translate(offset, glm::vec3(0, 0, -3.6f));
    SphereCollider &enemyBallCol =
        enemyShootTrigger.AddComponentAs<Collider, SphereCollider>(1.1f, offset,
                                                                   true);
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
    sphere.AddComponent<MeshRenderer>(ballModel, defaultShader);
    sphere.GetComponent<Transform>()->setScale(glm::vec3(2.5f));
    sphere.GetComponent<Transform>()->setPosition(arena.m_arenaOffset +
                                                  glm::vec3(0.0f, 5.0f, 0.0f));

    SphereCollider &ballCol =
        sphere.AddComponentAs<Collider, SphereCollider>(2.0f);
    ballCol.m_restitution = 0.6f;
    ballCol.m_friction = 1.0f;
    ballCol.m_layer = CAT_BALL;
    ballCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_ENEMY | CAT_GROUND;

    sphere.AddComponent<Rigidbody>(1.0f, 0.3f, 30.0f, 0.8f, 0.8f);
    Rigidbody *sphereRb = sphere.GetComponent<Rigidbody>();
    sphereRb->m_invInertiaTensor =
        Rigidbody::createSphereInverseInertiaTensor(10.0f, 2.0f);
    sphere.AddComponent<Ball>();

    arena.m_playerA = &player;
    arena.m_playerB = &enemy;
    arena.m_ball = &sphere;

    enemyA.init(&enemy, &sphere, arena.m_gateAPos, arena.m_gateBPos);
    enemyB.init(&player, &sphere, arena.m_gateBPos, arena.m_gateAPos);

    enemyA.onKickReward = [this, currentId](float reward)
    { this->m_arenas[currentId].onKickA(reward); };
    enemyB.onKickReward = [this, currentId](float reward)
    { this->m_arenas[currentId].onKickB(reward); };
}

void TrainerScene::update(float deltaTime) { Scene::update(deltaTime); }

void TrainerScene::fixedUpdate(float deltaTime)
{
    static constexpr float gravity = 42.0f;

#pragma omp parallel for schedule(static, 16)
    for (int i = 0; i < m_arenas.size(); i++)
    {
        m_arenas[i].step(deltaTime, gravity);
    }
}

void TrainerScene::draw()
{
    BaseScene::draw();

    for (auto &arena : m_arenas)
    {
        m_renderSystem->render(arena.m_entities, m_mainCamera->m_entity);
    }
}
