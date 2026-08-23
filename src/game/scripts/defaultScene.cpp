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
#include "scripts/powerup.hpp"
#include "scripts/shoeController.hpp"
#include "world/components/collider.hpp"
#include "world/components/light.hpp"
#include "world/components/meshRenderer.hpp"
#include "world/components/rigidbody.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"
#include "world/scene.hpp"

DefaultScene::DefaultScene(unsigned int whiteTextureId, int playerCharIdx,
                           int enemyCharIdx)
    : BaseScene(whiteTextureId), m_playerCharIdx(playerCharIdx),
      m_enemyCharIdx(enemyCharIdx)
{
}

DefaultScene::~DefaultScene()
{
    if (m_isSoundInitialized)
    {
        ma_sound_uninit(&m_matchMusic);
    }

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

    ma_result result = ma_sound_init_from_file(
        &app.m_soundEngine, "assets/sounds/crowdBackground.mp3",
        MA_SOUND_FLAG_STREAM, nullptr, nullptr, &m_matchMusic);
    if (result == MA_SUCCESS)
    {
        m_isSoundInitialized = true;
        ma_sound_set_looping(&m_matchMusic, MA_TRUE);
        ma_sound_start(&m_matchMusic);
    }

    Shader *defaultShader = app.getShader("default");

    Entity &player = createEntity();
    player.AddComponent<Transform>(glm::vec3(0, 10, 50), glm::vec3(0),
                                   glm::vec3(1.5f));

    std::shared_ptr<Model> playerModel =
        std::make_shared<Model>(CHARACTERS[m_playerCharIdx].modelPath);

    std::shared_ptr<Model> enemyModel =
        std::make_shared<Model>(CHARACTERS[m_enemyCharIdx].modelPath);

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
    Footballer &playerFb = player.AddComponent<Footballer>();
    playerFb.m_speed = CHARACTERS[m_playerCharIdx].speed;
    playerFb.m_jumpHeight = CHARACTERS[m_playerCharIdx].jumpHeight;
    playerFb.m_kickStrength = CHARACTERS[m_playerCharIdx].kickStrength;

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
    std::vector<uint32_t> layersSizes{43, 128, 128, 6};
    NeuralAgent &agent = enemy.AddComponent<NeuralAgent>(layersSizes);
    if (app.m_args.size() >= 2 && app.m_args[1] == "genetic")
    {
        std::cout << "Using genetic AI\n";
        agent.loadFromFile("best_brain.txt", true);
    }
    else
    {
        std::cout << "Using PPO AI\n";
        agent.loadFromFile("ppo_brain.txt", false);
    }

    enemy.AddComponent<MeshRenderer>(enemyModel, defaultShader);
    enemy.AddComponent<Rigidbody>(10.0f, 0.1f, 0.5f, 0.99f, 0.99f);
    SphereCollider &enemyCol =
        enemy.AddComponentAs<Collider, SphereCollider>(1.5f);
    enemyCol.m_restitution = 0.0f;
    enemyCol.m_layer = CAT_ENEMY;
    enemyCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_GROUND;
    enemy.GetComponent<Rigidbody>()->m_invInertiaTensor =
        Rigidbody::createSphereInverseInertiaTensor(1.0f, 2.0f);
    Footballer &enemyFb = enemy.AddComponent<Footballer>();
    enemyFb.m_speed = CHARACTERS[m_enemyCharIdx].speed;
    enemyFb.m_jumpHeight = CHARACTERS[m_enemyCharIdx].jumpHeight;
    enemyFb.m_kickStrength = CHARACTERS[m_enemyCharIdx].kickStrength;

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

    sphere.AddComponent<Rigidbody>(1.0f, 0.3f, 30.0f, 0.8f, 0.8f);
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

    m_gameState = GameState::Countdown;
    m_stateTimer = 3.0f;
    if (m_player)
        m_player->GetComponent<Footballer>()->canMove = false;
    if (m_enemy)
        m_enemy->GetComponent<Footballer>()->canMove = false;

    generatePowerups(&cameraPlayer);

    std::cout << "Scene initialized successfully\n";
}

void DefaultScene::update(float deltaTime)
{
    Application &app = Application::Get();
    if (app.GetInput().isKeyPressed(GLFW_KEY_ESCAPE))
    {
        if (m_gameState != GameState::GameFinished)
        {
            m_isPaused = !m_isPaused;
            if (m_isPaused)
            {
                if (m_isSoundInitialized)
                {
                    ma_sound_stop(&m_matchMusic);
                }
                glfwSetInputMode(app.m_window->getNativeWindow(), GLFW_CURSOR,
                                 GLFW_CURSOR_NORMAL);
            }
            else
            {
                if (m_isSoundInitialized)
                {
                    ma_sound_start(&m_matchMusic);
                }
                glfwSetInputMode(app.m_window->getNativeWindow(), GLFW_CURSOR,
                                 GLFW_CURSOR_DISABLED);
            }
        }
    }

    if (m_isPaused)
    {
        return;
    }

    Scene::update(deltaTime);

    if (m_gameState == GameState::Playing)
    {
        if (m_matchTimer > 0.0f)
        {
            m_matchTimer -= deltaTime;
            if (m_matchTimer < 0.0f)
            {
                m_matchTimer = 0.0f;
            }
        }

        if (m_matchTimer == 0.0f)
        {
            if (m_playerScore != m_enemyScore)
            {
                m_gameState = GameState::GameFinished;
                Application &app = Application::Get();
                glfwSetInputMode(app.m_window->getNativeWindow(), GLFW_CURSOR,
                                 GLFW_CURSOR_NORMAL);
                if (m_player)
                    m_player->GetComponent<Footballer>()->canMove = false;
                if (m_enemy)
                    m_enemy->GetComponent<Footballer>()->canMove = false;
            }
        }
    }
    else if (m_gameState == GameState::GoalScored)
    {
        m_stateTimer -= deltaTime;
        if (m_stateTimer <= 0.0f)
        {
            if (m_matchTimer == 0.0f) // Golden goal scored
            {
                m_gameState = GameState::GameFinished;
                Application &app = Application::Get();
                glfwSetInputMode(app.m_window->getNativeWindow(), GLFW_CURSOR,
                                 GLFW_CURSOR_NORMAL);
                if (m_player)
                    m_player->GetComponent<Footballer>()->canMove = false;
                if (m_enemy)
                    m_enemy->GetComponent<Footballer>()->canMove = false;
            }
            else
            {
                resetPositions();
                m_gameState = GameState::Countdown;
                m_stateTimer = 3.0f;
                if (m_player)
                    m_player->GetComponent<Footballer>()->canMove = false;
                if (m_enemy)
                    m_enemy->GetComponent<Footballer>()->canMove = false;
            }
        }
    }
    else if (m_gameState == GameState::Countdown)
    {
        m_stateTimer -= deltaTime;
        if (m_stateTimer <= 0.0f)
        {
            Application &app = Application::Get();
            ma_engine_play_sound(&app.m_soundEngine,
                                 "assets/sounds/refWhistle.mp3", NULL);
            m_gameState = GameState::Playing;
            if (m_player)
                m_player->GetComponent<Footballer>()->canMove = true;
            if (m_enemy)
                m_enemy->GetComponent<Footballer>()->canMove = true;
        }
    }
}

void DefaultScene::fixedUpdate(float deltaTime)
{
    if (m_isPaused)
    {
        return;
    }
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
    if (m_matchTimer == 0.0f && m_gameState == GameState::Playing)
    {
        float textWidth = ImGui::CalcTextSize("GOLDEN GOAL!").x;
        ImGui::SetCursorPosX((windowSize.x - textWidth) * 0.5f);
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "GOLDEN GOAL!");
    }
    else
    {
        float textWidth = ImGui::CalcTextSize("00:00").x;
        ImGui::SetCursorPosX((windowSize.x - textWidth) * 0.5f);
        int totalSeconds = static_cast<int>(m_matchTimer);
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        ImGui::Text("%02d:%02d", minutes, seconds);
    }

    ImGui::End();

    if (m_isPaused)
    {
        Application &app = Application::Get();
        ImGui::SetNextWindowPos(
            ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400.0f, 350.0f), ImGuiCond_Always);

        ImGuiWindowFlags overlayFlags =
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushStyleColor(ImGuiCol_WindowBg,
                              ImVec4(0.05f, 0.05f, 0.08f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.5f, 1.0f, 0.8f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

        ImGui::Begin("PauseMenuWindow", nullptr, overlayFlags);

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        ImGui::SetWindowFontScale(2.2f);
        std::string pauseText = "GAME PAUSED";
        float tw = ImGui::CalcTextSize(pauseText.c_str()).x;
        ImGui::SetCursorPosX((400.0f - tw) * 0.5f);
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%s",
                           pauseText.c_str());
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Dummy(ImVec2(0.0f, 25.0f));

        float buttonWidth = 220.0f;
        float buttonHeight = 45.0f;

        // Resume button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.6f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.08f, 0.5f, 0.15f, 1.0f));
        ImGui::SetCursorPosX((400.0f - buttonWidth) * 0.5f);
        if (ImGui::Button("RESUME", ImVec2(buttonWidth, buttonHeight)))
        {
            m_isPaused = false;
            if (m_isSoundInitialized)
            {
                ma_sound_start(&m_matchMusic);
            }
            glfwSetInputMode(app.m_window->getNativeWindow(), GLFW_CURSOR,
                             GLFW_CURSOR_DISABLED);
        }
        ImGui::PopStyleColor(3);

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        // Restart button
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.15f, 0.15f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.25f, 0.25f, 0.35f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.1f, 0.1f, 0.15f, 1.0f));
        ImGui::SetCursorPosX((400.0f - buttonWidth) * 0.5f);
        if (ImGui::Button("RESTART MATCH", ImVec2(buttonWidth, buttonHeight)))
        {
            Application &app = Application::Get();
            app.loadScene(std::make_unique<DefaultScene>(
                app.getWhiteTexture(), m_playerCharIdx, m_enemyCharIdx));
        }
        ImGui::PopStyleColor(3);

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        // Main Menu button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.3f, 0.5f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.1f, 0.3f, 0.7f, 1.0f));
        ImGui::SetCursorPosX((400.0f - buttonWidth) * 0.5f);
        if (ImGui::Button("MAIN MENU", ImVec2(buttonWidth, buttonHeight)))
        {
            Application &app = Application::Get();
            app.loadScene(std::make_unique<MenuScene>(app.getWhiteTexture()));
        }
        ImGui::PopStyleColor(3);

        ImGui::End();

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(3);
    }
    else if (m_gameState == GameState::GoalScored)
    {
        ImGui::SetNextWindowPos(
            ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.3f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin("GoalText", nullptr, flags);
        ImGui::SetWindowFontScale(4.0f);
        float tw = ImGui::CalcTextSize(m_goalText.c_str()).x;
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - tw) * 0.5f);
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%s",
                           m_goalText.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::End();
    }
    else if (m_gameState == GameState::Countdown)
    {
        ImGui::SetNextWindowPos(
            ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin("CountdownText", nullptr, flags);
        ImGui::SetWindowFontScale(6.0f);
        int displayTime = static_cast<int>(std::ceil(m_stateTimer));
        if (displayTime > 0)
        {
            std::string cdText = std::to_string(displayTime);
            float tw = ImGui::CalcTextSize(cdText.c_str()).x;
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - tw) * 0.5f);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s",
                               cdText.c_str());
        }
        ImGui::SetWindowFontScale(1.0f);
        ImGui::End();
    }
    else if (m_gameState == GameState::GameFinished)
    {
        ImGui::SetNextWindowPos(
            ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400.0f, 320.0f), ImGuiCond_Always);

        ImGuiWindowFlags overlayFlags =
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushStyleColor(ImGuiCol_WindowBg,
                              ImVec4(0.05f, 0.05f, 0.08f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.4f, 0.8f, 0.7f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

        ImGui::Begin("GameFinishedWindow", nullptr, overlayFlags);

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        ImGui::SetWindowFontScale(2.2f);
        std::string winText = "";
        ImVec4 winColor;
        if (m_playerScore > m_enemyScore)
        {
            winText = "VICTORY!";
            winColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f); // Green
        }
        else
        {
            winText = "DEFEAT!";
            winColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f); // Red
        }

        float tw = ImGui::CalcTextSize(winText.c_str()).x;
        ImGui::SetCursorPosX((400.0f - tw) * 0.5f);
        ImGui::TextColored(winColor, "%s", winText.c_str());
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        // Match Result
        ImGui::SetWindowFontScale(1.5f);
        std::string scoreText = "Score: " + std::to_string(m_playerScore) +
                                " - " + std::to_string(m_enemyScore);
        float sw = ImGui::CalcTextSize(scoreText.c_str()).x;
        ImGui::SetCursorPosX((400.0f - sw) * 0.5f);
        ImGui::Text("%s", scoreText.c_str());
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Dummy(ImVec2(0.0f, 35.0f));

        float buttonWidth = 220.0f;
        float buttonHeight = 45.0f;

        // Restart button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.6f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.08f, 0.5f, 0.15f, 1.0f));
        ImGui::SetCursorPosX((400.0f - buttonWidth) * 0.5f);
        if (ImGui::Button("RESTART MATCH", ImVec2(buttonWidth, buttonHeight)))
        {
            Application &app = Application::Get();
            app.loadScene(std::make_unique<DefaultScene>(
                app.getWhiteTexture(), m_playerCharIdx, m_enemyCharIdx));
        }
        ImGui::PopStyleColor(3);

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        // Main Menu button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.3f, 0.5f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.1f, 0.3f, 0.7f, 1.0f));
        ImGui::SetCursorPosX((400.0f - buttonWidth) * 0.5f);
        if (ImGui::Button("MAIN MENU", ImVec2(buttonWidth, buttonHeight)))
        {
            Application &app = Application::Get();
            app.loadScene(std::make_unique<MenuScene>(app.getWhiteTexture()));
        }
        ImGui::PopStyleColor(3);

        ImGui::End();

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(3);
    }
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
        if (m_gameState != GameState::Playing)
            return;
        Application &app = Application::Get();
        ma_engine_play_sound(&app.m_soundEngine, "assets/sounds/crowdCheer.mp3",
                             NULL);
        m_enemyScore++;
        std::cout << "GOAL for ENEMY! Score is: " << m_playerScore << " - "
                  << m_enemyScore << "\n";
        m_gameState = GameState::GoalScored;
        m_stateTimer = 3.0f;
        if (m_matchTimer == 0.0f)
        {
            m_goalText = "GOLDEN GOAL! ENEMY WINS!";
        }
        else
        {
            m_goalText = "ENEMY SCORED!";
        }

        for (auto &entity : m_entities)
        {
            Powerup *powerup = entity->GetComponent<Powerup>();
            if (powerup)
            {
                powerup->reset();
            }
        }
    };
    auto onGoalB = [this]()
    {
        if (m_gameState != GameState::Playing)
            return;
        Application &app = Application::Get();
        ma_engine_play_sound(&app.m_soundEngine, "assets/sounds/crowdCheer.mp3",
                             NULL);
        m_playerScore++;
        std::cout << "GOAL for PLAYER! Score is: " << m_playerScore << " - "
                  << m_enemyScore << "\n";
        m_gameState = GameState::GoalScored;
        m_stateTimer = 3.0f;
        if (m_matchTimer == 0.0f)
        {
            m_goalText = "GOLDEN GOAL! PLAYER WINS!";
        }
        else
        {
            m_goalText = "PLAYER SCORED!";
        }

        for (auto &entity : m_entities)
        {
            Powerup *powerup = entity->GetComponent<Powerup>();
            if (powerup)
            {
                powerup->reset();
            }
        }
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

        Footballer *fb = m_player->GetComponent<Footballer>();
        if (fb)
        {
            fb->m_lastRotation = glm::vec2(0.0f, 0.0f);
        }
        PlayerController *pc = m_player->GetComponent<PlayerController>();
        if (pc)
        {
            pc->resetRotation(0.0f, 0.0f);
        }
    }
    if (m_enemy)
    {
        m_enemy->GetComponent<Transform>()->setPosition(glm::vec3(0, 2, -50));
        m_enemy->GetComponent<Transform>()->setRotation(
            glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
        m_enemy->GetComponent<Rigidbody>()->m_velocity = glm::vec3(0);
        m_enemy->GetComponent<Rigidbody>()->m_angularVelocity = glm::vec3(0);

        Footballer *fb = m_enemy->GetComponent<Footballer>();
        if (fb)
        {
            fb->m_lastRotation = glm::vec2(0.0f, glm::radians(180.0f));
        }
        EnemyController *ec = m_enemy->GetComponent<EnemyController>();
        if (ec)
        {
            ec->m_yaw = glm::radians(180.0f);
            ec->m_pitch = 0.0f;
            ec->m_lastTurnYaw = 0.0f;
            ec->m_lastTurnPitch = 0.0f;
        }
    }
    if (m_ball)
    {
        m_ball->GetComponent<Transform>()->setPosition(glm::vec3(0, 5, 0));
        m_ball->GetComponent<Rigidbody>()->m_velocity = glm::vec3(0);
        m_ball->GetComponent<Rigidbody>()->m_angularVelocity = glm::vec3(0);
    }
}

void DefaultScene::generatePowerups(Entity *cameraPlayer)
{
    Application &app = Application::Get();
    Shader *unlitShader = app.getShader("unlit");

    std::shared_ptr<Model> highJumpPowerupModel =
        std::make_shared<Model>("assets/models/highJumpPowerup.obj");

    std::shared_ptr<Model> lowJumpPowerupModel =
        std::make_shared<Model>("assets/models/lowJumpPowerup.obj");

    std::shared_ptr<Model> strongShotPowerupModel =
        std::make_shared<Model>("assets/models/strongShotPowerup.obj");

    std::shared_ptr<Model> weakShotPowerupModel =
        std::make_shared<Model>("assets/models/weakShotPowerup.obj");

    std::shared_ptr<Model> highSpeedPowerupModel =
        std::make_shared<Model>("assets/models/highSpeedPowerup.obj");

    std::shared_ptr<Model> slowSpeedPowerupModel =
        std::make_shared<Model>("assets/models/slowSpeedPowerup.obj");

    float powerupScale = 3.2f;

    // 1. Speed Boost Powerup
    Entity &speedPowerup = createEntity();
    speedPowerup.AddComponent<Transform>(glm::vec3(0, 3.0f, 0), glm::vec3(0),
                                         glm::vec3(powerupScale));
    speedPowerup.AddComponent<MeshRenderer>(highSpeedPowerupModel, unlitShader);
    SphereCollider &speedCol =
        speedPowerup.AddComponentAs<Collider, SphereCollider>(
            powerupScale, glm::mat4(1.0f), true);
    speedCol.m_layer = CAT_GROUND;
    speedCol.m_mask = CAT_BALL;
    speedPowerup.AddComponent<Powerup>(PowerupType::SpeedBoost, 5.0f, 8.0f)
        .setTargetEntity(cameraPlayer);

    // 2. Speed Debuff Powerup
    Entity &slowPowerup = createEntity();
    slowPowerup.AddComponent<Transform>(glm::vec3(0, 3.0f, 0), glm::vec3(0),
                                        glm::vec3(2.5f));
    slowPowerup.AddComponent<MeshRenderer>(slowSpeedPowerupModel, unlitShader);
    SphereCollider &slowCol =
        slowPowerup.AddComponentAs<Collider, SphereCollider>(
            2.5f, glm::mat4(1.0f), true);
    slowCol.m_layer = CAT_GROUND;
    slowCol.m_mask = CAT_BALL;
    slowPowerup.AddComponent<Powerup>(PowerupType::SpeedDebuff, 5.0f, 8.0f)
        .setTargetEntity(cameraPlayer);

    // 3. Super Kick Powerup
    std::shared_ptr<Model> redBallModel =
        std::make_shared<Model>("assets/models/redBall.obj");
    Entity &kickPowerup = createEntity();
    kickPowerup.AddComponent<Transform>(glm::vec3(0, 3.0f, 0), glm::vec3(0),
                                        glm::vec3(2.5f));
    kickPowerup.AddComponent<MeshRenderer>(strongShotPowerupModel, unlitShader);
    SphereCollider &kickCol =
        kickPowerup.AddComponentAs<Collider, SphereCollider>(
            2.5f, glm::mat4(1.0f), true);
    kickCol.m_layer = CAT_GROUND;
    kickCol.m_mask = CAT_BALL;
    kickPowerup.AddComponent<Powerup>(PowerupType::SuperKick, 5.0f, 8.0f)
        .setTargetEntity(cameraPlayer);

    // 4. Weak Kick Powerup
    Entity &weakKickPowerup = createEntity();
    weakKickPowerup.AddComponent<Transform>(glm::vec3(0, 3.0f, 0), glm::vec3(0),
                                            glm::vec3(2.5f));
    weakKickPowerup.AddComponent<MeshRenderer>(weakShotPowerupModel,
                                               unlitShader);
    SphereCollider &weakKickCol =
        weakKickPowerup.AddComponentAs<Collider, SphereCollider>(
            2.5f, glm::mat4(1.0f), true);
    weakKickCol.m_layer = CAT_GROUND;
    weakKickCol.m_mask = CAT_BALL;
    weakKickPowerup.AddComponent<Powerup>(PowerupType::WeakKick, 5.0f, 8.0f)
        .setTargetEntity(cameraPlayer);

    // 5. Super Jump Powerup
    Entity &jumpPowerup = createEntity();
    jumpPowerup.AddComponent<Transform>(glm::vec3(0, 3.0f, 0), glm::vec3(0),
                                        glm::vec3(2.5f));
    jumpPowerup.AddComponent<MeshRenderer>(highJumpPowerupModel, unlitShader);
    SphereCollider &jumpCol =
        jumpPowerup.AddComponentAs<Collider, SphereCollider>(
            2.5f, glm::mat4(1.0f), true);
    jumpCol.m_layer = CAT_GROUND;
    jumpCol.m_mask = CAT_BALL;
    jumpPowerup.AddComponent<Powerup>(PowerupType::SuperJump, 5.0f, 8.0f)
        .setTargetEntity(cameraPlayer);

    // 6. Low Jump Powerup
    Entity &lowJumpPowerup = createEntity();
    lowJumpPowerup.AddComponent<Transform>(glm::vec3(0, 3.0f, 0), glm::vec3(0),
                                           glm::vec3(2.5f));
    lowJumpPowerup.AddComponent<MeshRenderer>(lowJumpPowerupModel, unlitShader);
    SphereCollider &lowJumpCol =
        lowJumpPowerup.AddComponentAs<Collider, SphereCollider>(
            2.5f, glm::mat4(1.0f), true);
    lowJumpCol.m_layer = CAT_GROUND;
    lowJumpCol.m_mask = CAT_BALL;
    lowJumpPowerup.AddComponent<Powerup>(PowerupType::LowJump, 5.0f, 8.0f)
        .setTargetEntity(cameraPlayer);
}
