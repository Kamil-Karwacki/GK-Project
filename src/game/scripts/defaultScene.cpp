#include "defaultScene.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <memory>

#include "menuScene.hpp"

#include "ai/neuralAgent.hpp"
#include "cameraController.hpp"
#include "core/application.hpp"
#include "footballer.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
#include "graphics/model.hpp"
#include "imgui.h"
#include "playerController.hpp"
#include "scripts/ai/neuralAgent.hpp"
#include "scripts/ball.hpp"
#include "scripts/enemyController.hpp"
#include "scripts/footballerShootTrigger.hpp"
#include "scripts/pitchGenerator.hpp"
#include "scripts/playerGrounded.hpp"
#include "scripts/shoeController.hpp"
#include "world/components/collider.hpp"
#include "world/components/light.hpp"
#include "world/components/meshRenderer.hpp"
#include "world/components/rigidbody.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"
#include "world/scene.hpp"

DefaultScene::~DefaultScene()
{
    if (m_skyboxVAO != 0)
    {
        glDeleteVertexArrays(1, &m_skyboxVAO);
    }
    if (m_skyboxVBO != 0)
    {
        glDeleteBuffers(1, &m_skyboxVBO);
    }
}

void DefaultScene::init()
{
    generateTerrain();

    Application &app = Application::Get();
    glfwSetInputMode(app.m_window->getNativeWindow(), GLFW_CURSOR,
                     GLFW_CURSOR_DISABLED);

    Shader *defaultShader = app.getShader("default");

    Entity &player = createEntity();
    player.AddComponent<Transform>(glm::vec3(0, 10, 50), glm::vec3(0),
                                   glm::vec3(1.5f));

    std::shared_ptr<Model> playerModel =
        std::make_shared<Model>("assets/models/footballer1.obj");

    std::shared_ptr<Model> enemyModel =
        std::make_shared<Model>("assets/models/footballer2.obj");

    std::shared_ptr<Model> ballModel =
        std::make_shared<Model>("assets/models/ball.obj");

    std::shared_ptr<Model> shoeModel =
        std::make_shared<Model>("assets/models/shoe.obj");

    player.AddComponent<PlayerController>();
    player.AddComponent<MeshRenderer>(playerModel, defaultShader);
    player.AddComponent<Rigidbody>(10.0f, 0.1f, 0.5f, 0.99f, 0.99f);
    auto &playerCol = player.AddComponentAs<Collider, SphereCollider>(1.5f);
    playerCol.m_restitution = 0.0f;
    playerCol.m_layer = CAT_PLAYER;
    playerCol.m_mask = CAT_BALL | CAT_ENEMY | CAT_GROUND;
    player.GetComponent<Rigidbody>()->m_invInertiaTensor =
        Rigidbody::createSphereInverseInertiaTensor(1.0f, 2.0f);
    player.AddComponent<Footballer>();

    Entity &playerShootTrigger = createEntity();
    playerShootTrigger.AddComponent<Transform>();
    playerShootTrigger.AddComponent<FootballerShootTrigger>(&player);
    glm::mat4 offset = glm::mat4(1.0f);
    offset = glm::translate(offset, glm::vec3(0, 0, -3.6f));
    SphereCollider &playerBallCol =
        playerShootTrigger.AddComponentAs<Collider, SphereCollider>(
            1.1f, offset, true);
    playerBallCol.m_layer = CAT_PLAYER;
    playerBallCol.m_mask = CAT_BALL;

    Entity &playerGrounded = createEntity();
    playerGrounded.AddComponent<Transform>();
    SphereCollider &groundCol =
        playerGrounded.AddComponentAs<Collider, SphereCollider>(
            0.5f, glm::mat4(1.0f), true);
    groundCol.m_layer = CAT_PLAYER;
    groundCol.m_mask = CAT_GROUND | CAT_ENEMY;
    playerGrounded.AddComponent<PlayerGrounded>(&player,
                                                glm::vec3(0, -1.1f, 0));

    Entity &cameraPlayer = createEntity();
    cameraPlayer.AddComponent<Transform>();
    m_mainCamera = &cameraPlayer.AddComponent<Camera>();
    cameraPlayer.AddComponent<CameraController>(&player);

    Entity &playerShoe = createEntity();
    playerShoe.AddComponent<Transform>();
    playerShoe.AddComponent<MeshRenderer>(shoeModel, defaultShader);
    playerShoe.AddComponent<ShoeController>(glm::vec3(0, 0, 2), &player);
    player.GetComponent<Footballer>()->m_shoe = &playerShoe;

    Entity &enemy = createEntity();
    enemy.AddComponent<Transform>(glm::vec3(0, 10, -50),
                                  glm::vec3(0, glm::radians(180.0f), 0),
                                  glm::vec3(1.5f));
    EnemyController &enemyController = enemy.AddComponent<EnemyController>();
    NeuralAgent &agent = enemy.AddComponent<NeuralAgent>(43, 256, 6);
    agent.loadFromFile("best_brain.txt");
    enemy.AddComponent<MeshRenderer>(enemyModel, defaultShader);
    enemy.AddComponent<Rigidbody>(10.0f, 0.1f, 0.5f, 0.99f, 0.99f);
    SphereCollider &enemyCol =
        enemy.AddComponentAs<Collider, SphereCollider>(1.5f);
    enemyCol.m_restitution = 0.0f;
    enemyCol.m_layer = CAT_ENEMY;
    enemyCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_GROUND;
    enemy.GetComponent<Rigidbody>()->m_invInertiaTensor =
        Rigidbody::createSphereInverseInertiaTensor(1.0f, 2.0f);
    enemy.AddComponent<Footballer>();

    Entity &enemyShootTrigger = createEntity();
    enemyShootTrigger.AddComponent<Transform>();
    enemyShootTrigger.AddComponent<FootballerShootTrigger>(&enemy);
    offset = glm::mat4(1.0f);
    offset = glm::translate(offset, glm::vec3(0, 0, -3.6f));
    SphereCollider &enemyBallCol =
        enemyShootTrigger.AddComponentAs<Collider, SphereCollider>(1.1f, offset,
                                                                   true);

    enemyBallCol.m_layer = CAT_PLAYER;
    enemyBallCol.m_mask = CAT_BALL;

    Entity &enemyGrounded = createEntity();
    enemyGrounded.AddComponent<Transform>();
    SphereCollider &enemyGroundCol =
        enemyGrounded.AddComponentAs<Collider, SphereCollider>(
            0.5f, glm::mat4(1.0f), true);
    enemyGroundCol.m_layer = CAT_ENEMY;
    enemyGroundCol.m_mask = CAT_GROUND | CAT_BALL | CAT_PLAYER;
    enemyGrounded.AddComponent<PlayerGrounded>(&enemy, glm::vec3(0, -1.1f, 0));

    Entity &enemyShoe = createEntity();
    enemyShoe.AddComponent<Transform>();
    enemyShoe.AddComponent<MeshRenderer>(shoeModel, defaultShader);
    enemyShoe.AddComponent<ShoeController>(glm::vec3(0, 0, 2), &enemy);
    enemy.GetComponent<Footballer>()->m_shoe = &enemyShoe;

    Entity &sphere = createEntity();
    sphere.AddComponent<Transform>();
    sphere.AddComponent<MeshRenderer>(ballModel, defaultShader);
    sphere.GetComponent<Transform>()->setScale(glm::vec3(2.5f));
    sphere.GetComponent<Transform>()->setPosition(glm::vec3(0, 5.0f, 0.0f));
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

    Entity &sun = createEntity();
    sun.AddComponent<Transform>(glm::vec3(0, 50, 0),
                                glm::vec3(glm::radians(290.0f), 0, 0));
    sun.AddComponent<DirectionalLight>(glm::vec3(1.0f, 1.0f, 1.0f), 0.3f, 0.6f,
                                       0.4f);

    enemyController.init(&player, &sphere, m_enemyGatePos, m_playerGatePos);

    float skyboxVertices[] = {
        -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

        -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
        -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

        1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

    glGenVertexArrays(1, &m_skyboxVAO);
    glGenBuffers(1, &m_skyboxVBO);
    glBindVertexArray(m_skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glBindVertexArray(0);

    m_player = &player;
    m_enemy = &enemy;
    m_ball = &sphere;

    std::cout << "Scene initialized successfully\n";
}

void DefaultScene::update(float deltaTime)
{
    Scene::update(deltaTime);

    m_matchTimer -= deltaTime;
    if (m_matchTimer < 0.0f)
    {
        m_matchTimer = 0.0f;
        Application &app = Application::Get();
        app.loadScene(std::make_unique<MenuScene>(app.getWhiteTexture()));
        return;
    }

    Application &app = Application::Get();
    if (app.GetInput().isKeyPressed(GLFW_KEY_ESCAPE))
    {
        app.loadScene(std::make_unique<MenuScene>(app.getWhiteTexture()));
    }
}

void DefaultScene::fixedUpdate(float deltaTime)
{
    static constexpr float gravity = 42.0f;
    for (auto &entity : m_entities)
    {
        Rigidbody *rb = entity->GetComponent<Rigidbody>();
        if (rb)
        {
            rb->m_forceAcc += glm::vec3(0.0f, -1.0f, 0.0f) * gravity *
                              (1.0f / rb->m_inverseMass);
        }
    }
    Scene::fixedUpdate(deltaTime);
}

void DefaultScene::draw()
{
    BaseScene::draw();

    Application &app = Application::Get();
    Shader *skyboxShader = app.getShader("skybox");
    if (skyboxShader && m_skyboxVAO != 0)
    {
        skyboxShader->use();

        glm::mat4 projection = getMainProjectionMatrix();
        glm::mat4 view = getMainViewMatrix();
        skyboxShader->setMat4("projection", 1, GL_FALSE, &projection[0][0]);
        skyboxShader->setMat4("view", 1, GL_FALSE, &view[0][0]);

        glm::vec3 sunDir(0.0f, 1.0f, 0.0f);
        for (const auto &entity : m_entities)
        {
            DirectionalLight *light = entity->GetComponent<DirectionalLight>();
            if (light)
            {
                Transform *transform = entity->GetComponent<Transform>();
                if (transform)
                {
                    sunDir = -transform->getFront();
                    break;
                }
            }
        }
        skyboxShader->setVec3("u_sunDir", sunDir);

        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);

        glBindVertexArray(m_skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }
}

void DefaultScene::drawUI()
{
    ImGuiIO &io = ImGui::GetIO();

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, 10.0f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.65f);

    ImGui::Begin("GameScoreboard", nullptr, flags);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15.0f, 0.0f));

    ImGui::TextColored(ImVec4(0.3f, 0.6f, 1.0f, 1.0f), "PLAYER");

    ImGui::SameLine();
    ImGui::SetWindowFontScale(3.0f);

    ImGui::Text("%d", m_playerScore);
    ImGui::SameLine();
    ImGui::Text(":");
    ImGui::SameLine();
    ImGui::Text("%d", m_enemyScore);

    ImGui::SetWindowFontScale(1.0f);
    ImGui::SameLine();

    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "ENEMY");

    ImGui::PopStyleVar();

    ImVec2 windowSize = ImGui::GetWindowSize();
    float textWidth = ImGui::CalcTextSize("00:00").x;
    ImGui::SetCursorPosX((windowSize.x - textWidth) * 0.5f);

    int totalSeconds = static_cast<int>(m_matchTimer);
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    ImGui::Text("%02d:%02d", minutes, seconds);

    ImGui::End();
}

void DefaultScene::generateTerrain()
{
    Application &app = Application::Get();
    Shader *defaultShader = app.getShader("default");

    glm::vec2 pitchSize = glm::vec2(115, 74);
    pitchSize *= 1.4f;

    glm::vec2 tribuneOffset = glm::vec2(0, 10);
    float tribuneLength = 32.0f;

    PitchGenerator::PitchConfig config;
    config.gateSize = glm::vec3(30.0f, 11.0f, 7.0f);
    config.pitchSize = pitchSize;

    PitchGenerator::generatePitch(m_entities, this, glm::vec3(0.0f),
                                  defaultShader, config);

    auto onGoalA = [this]()
    {
        m_enemyScore++;
        std::cout << "GOAL for ENEMY! Score is: " << m_playerScore << " - "
                  << m_enemyScore << "\n";
        this->resetPositions();
    };
    auto onGoalB = [this]()
    {
        m_playerScore++;
        std::cout << "GOAL for PLAYER! Score is: " << m_playerScore << " - "
                  << m_enemyScore << "\n";
        this->resetPositions();
    };

    auto gatesInfo =
        PitchGenerator::generateGates(m_entities, this, glm::vec3(0.0f),
                                      defaultShader, onGoalA, onGoalB, config);

    m_playerGatePos = gatesInfo.gateAPos;
    m_enemyGatePos = gatesInfo.gateBPos;

    std::shared_ptr<Model> tribuneModel =
        std::make_shared<Model>("assets/models/tribune.obj");

    glm::vec2 tribuneCount = glm::round(pitchSize / tribuneLength);
    glm::vec2 tribunesLength = pitchSize / tribuneCount;
    glm::vec2 tribuneScale = tribunesLength / tribuneLength;

    for (size_t i = 0; i < tribuneCount.y; i++)
    {
        Entity &tribune = createEntity();
        tribune.AddComponent<Transform>(
            glm::vec3(pitchSize.y / 2.0f - tribunesLength.x / 2.0f -
                          tribunesLength.y * i,
                      0, pitchSize.x / 2.0f + tribuneOffset.y),
            glm::vec3(0, glm::radians(180.0f), 0),
            glm::vec3(tribuneScale.y, 1, 1));
        tribune.AddComponent<MeshRenderer>(tribuneModel, defaultShader);

        Entity &tribuneB = createEntity();
        tribuneB.AddComponent<Transform>(
            glm::vec3(pitchSize.y / 2.0f - tribunesLength.x / 2.0f -
                          tribunesLength.y * i,
                      0, -pitchSize.x / 2.0f - tribuneOffset.y),
            glm::vec3(0, 0, 0), glm::vec3(tribuneScale.y, 1, 1));
        tribuneB.AddComponent<MeshRenderer>(tribuneModel, defaultShader);
    }

    for (size_t i = 0; i < tribuneCount.x; i++)
    {
        Entity &tribuneA = createEntity();
        tribuneA.AddComponent<Transform>(
            glm::vec3(pitchSize.y / 2.0f + tribuneOffset.y, 0,
                      pitchSize.x / 2.0f - tribunesLength.x / 2.0f -
                          tribunesLength.x * i),
            glm::vec3(0, glm::radians(270.0f), 0),
            glm::vec3(tribuneScale.x, 1, 1));
        tribuneA.AddComponent<MeshRenderer>(tribuneModel, defaultShader);

        Entity &tribuneB = createEntity();
        tribuneB.AddComponent<Transform>(
            glm::vec3(-pitchSize.y / 2.0f - tribuneOffset.y, 0,
                      pitchSize.x / 2.0f - tribunesLength.x / 2.0f -
                          tribunesLength.x * i),
            glm::vec3(0, glm::radians(90.0f), 0),
            glm::vec3(tribuneScale.x, 1, 1));
        tribuneB.AddComponent<MeshRenderer>(tribuneModel, defaultShader);
    }
}

void DefaultScene::resetPositions()
{
    if (m_player)
    {
        m_player->GetComponent<Transform>()->setPosition(glm::vec3(0, 2, 50));
        m_player->GetComponent<Transform>()->setRotation(glm::vec3(0.0f));
        m_player->GetComponent<Rigidbody>()->m_velocity = glm::vec3(0);
        m_player->GetComponent<Rigidbody>()->m_angularVelocity = glm::vec3(0);
    }
    if (m_enemy)
    {
        m_enemy->GetComponent<Transform>()->setPosition(glm::vec3(0, 2, -50));
        m_enemy->GetComponent<Transform>()->setRotation(
            glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
        m_enemy->GetComponent<Rigidbody>()->m_velocity = glm::vec3(0);
        m_enemy->GetComponent<Rigidbody>()->m_angularVelocity = glm::vec3(0);
    }
    if (m_ball)
    {
        m_ball->GetComponent<Transform>()->setPosition(glm::vec3(0, 5, 0));
        m_ball->GetComponent<Rigidbody>()->m_velocity = glm::vec3(0);
        m_ball->GetComponent<Rigidbody>()->m_angularVelocity = glm::vec3(0);
    }
}
