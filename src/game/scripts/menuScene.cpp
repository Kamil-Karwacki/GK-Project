#include "menuScene.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "core/application.hpp"
#include "defaultScene.hpp"
#include "footballer.hpp"
#include "graphics/model.hpp"
#include "imgui.h"
#include "world/components/light.hpp"
#include "world/components/meshRenderer.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"

MenuScene::~MenuScene()
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

void MenuScene::init()
{
    Application &app = Application::Get();
    
    glfwSetInputMode(app.m_window->getNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    m_cameraEntity = &createEntity();
    m_cameraEntity->AddComponent<Transform>(glm::vec3(0, 5, 0), glm::vec3(0), glm::vec3(1.0f));
    m_mainCamera = &m_cameraEntity->AddComponent<Camera>();
    m_mainCamera->fov = 60.0f;

    Entity &sun = createEntity();
    sun.AddComponent<Transform>(glm::vec3(0, 50, 0),
                                glm::vec3(glm::radians(290.0f), 0, 0));
    sun.AddComponent<DirectionalLight>(glm::vec3(1.0f, 1.0f, 1.0f), 0.3f, 0.6f,
                                       0.4f);

    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &m_skyboxVAO);
    glGenBuffers(1, &m_skyboxVBO);
    glBindVertexArray(m_skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    for (int i = 0; i < 3; ++i) {
        m_characterModels[i] = std::make_shared<Model>(CHARACTERS[i].modelPath);
    }
    Shader *defaultShader = app.getShader("default");

    m_playerPreview = &createEntity();
    m_playerPreview->AddComponent<Transform>(glm::vec3(-3.5f, 3.0f, -8.0f), glm::vec3(0.0f, glm::radians(30.0f), 0.0f), glm::vec3(0.0f));
    m_playerPreview->AddComponent<MeshRenderer>(m_characterModels[0], defaultShader);

    m_enemyPreview = &createEntity();
    m_enemyPreview->AddComponent<Transform>(glm::vec3(3.5f, 3.0f, -8.0f), glm::vec3(0.0f, glm::radians(-30.0f), 0.0f), glm::vec3(0.0f));
    m_enemyPreview->AddComponent<MeshRenderer>(m_characterModels[0], defaultShader);

    std::cout << "MenuScene initialized successfully\n";
}

void MenuScene::update(float deltaTime)
{
    if (m_state == MenuState::Main)
    {
        m_cameraRotationAngle += 0.05f * deltaTime;
        if (m_cameraEntity)
        {
            Transform *trans = m_cameraEntity->GetComponent<Transform>();
            if (trans)
            {
                trans->setRotation(glm::vec3(0.0f, m_cameraRotationAngle, 0.0f));
            }
        }
        
        if (m_playerPreview)
        {
            m_playerPreview->GetComponent<Transform>()->setScale(glm::vec3(0.0f));
        }
        if (m_enemyPreview)
        {
            m_enemyPreview->GetComponent<Transform>()->setScale(glm::vec3(0.0f));
        }
    }
    else if (m_state == MenuState::CharacterSelect)
    {
        if (m_cameraEntity)
        {
            Transform *trans = m_cameraEntity->GetComponent<Transform>();
            if (trans)
            {
                trans->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
            }
        }

        if (m_playerPreview)
        {
            Transform *trans = m_playerPreview->GetComponent<Transform>();
            trans->setScale(glm::vec3(1.5f));
            trans->addRotation(glm::vec3(0.0f, deltaTime * 1.5f, 0.0f));
            MeshRenderer *mr = m_playerPreview->GetComponent<MeshRenderer>();
            if (mr) mr->m_model = m_characterModels[m_playerCharIdx];
        }
        if (m_enemyPreview)
        {
            Transform *trans = m_enemyPreview->GetComponent<Transform>();
            trans->setScale(glm::vec3(1.5f));
            trans->addRotation(glm::vec3(0.0f, deltaTime * 1.5f, 0.0f));
            MeshRenderer *mr = m_enemyPreview->GetComponent<MeshRenderer>();
            if (mr) mr->m_model = m_characterModels[m_enemyCharIdx];
        }
    }

    BaseScene::update(deltaTime);
}

void MenuScene::draw()
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

void MenuScene::drawUI()
{
    ImGuiIO &io = ImGui::GetIO();

    if (m_state == MenuState::Main)
    {
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400.0f, 300.0f), ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                 ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.08f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.4f, 0.8f, 0.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

        ImGui::Begin("MainMenu", nullptr, flags);

        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        ImGui::SetWindowFontScale(2.5f);
        float titleWidth = ImGui::CalcTextSize("HEAD SOCCER 3D").x;
        ImGui::SetCursorPosX((400.0f - titleWidth) * 0.5f);
        ImGui::TextColored(ImVec4(0.3f, 0.6f, 1.0f, 1.0f), "HEAD SOCCER 3D");
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Dummy(ImVec2(0.0f, 40.0f));

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.3f, 0.6f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.2f, 0.5f, 1.0f));

        float buttonWidth = 200.0f;
        float buttonHeight = 45.0f;
        
        ImGui::SetCursorPosX((400.0f - buttonWidth) * 0.5f);
        if (ImGui::Button("PLAY GAME", ImVec2(buttonWidth, buttonHeight)))
        {
            m_state = MenuState::CharacterSelect;
        }
        ImGui::PopStyleColor(3);

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.38f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));

        ImGui::SetCursorPosX((400.0f - buttonWidth) * 0.5f);
        if (ImGui::Button("EXIT", ImVec2(buttonWidth, buttonHeight)))
        {
            Application::Get().close();
        }
        ImGui::PopStyleColor(3);

        ImGui::End();

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(3);
    }
    else if (m_state == MenuState::CharacterSelect)
    {
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(600.0f, 500.0f), ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                 ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.08f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.4f, 0.8f, 0.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

        ImGui::Begin("CharacterSelect", nullptr, flags);

        ImGui::SetWindowFontScale(1.8f);
        float titleWidth = ImGui::CalcTextSize("SELECT CHARACTER").x;
        ImGui::SetCursorPosX((600.0f - titleWidth) * 0.5f);
        ImGui::TextColored(ImVec4(0.3f, 0.6f, 1.0f, 1.0f), "SELECT CHARACTER");
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        ImGui::Columns(2, nullptr, false);
        
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "PLAYER");
        for (int i = 0; i < 3; ++i)
        {
            if (m_playerCharIdx == i)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            
            if (ImGui::Button((std::string(CHARACTERS[i].name) + "##P").c_str(), ImVec2(150, 30)))
                m_playerCharIdx = i;
                
            ImGui::PopStyleColor();
        }
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::Text("Stats:");
        ImGui::Text("Speed: %.0f", CHARACTERS[m_playerCharIdx].speed);
        ImGui::Text("Jump: %.0f", CHARACTERS[m_playerCharIdx].jumpHeight);
        ImGui::Text("Kick: %.0f", CHARACTERS[m_playerCharIdx].kickStrength);

        ImGui::NextColumn();

        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "AI OPPONENT");
        for (int i = 0; i < 3; ++i)
        {
            if (m_enemyCharIdx == i)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            
            if (ImGui::Button((std::string(CHARACTERS[i].name) + "##E").c_str(), ImVec2(150, 30)))
                m_enemyCharIdx = i;
                
            ImGui::PopStyleColor();
        }
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::Text("Stats:");
        ImGui::Text("Speed: %.0f", CHARACTERS[m_enemyCharIdx].speed);
        ImGui::Text("Jump: %.0f", CHARACTERS[m_enemyCharIdx].jumpHeight);
        ImGui::Text("Kick: %.0f", CHARACTERS[m_enemyCharIdx].kickStrength);

        ImGui::Columns(1);

        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 5.0f));

        ImGui::SetWindowFontScale(1.2f);
        float controlsTitleWidth = ImGui::CalcTextSize("CONTROLS").x;
        ImGui::SetCursorPosX((600.0f - controlsTitleWidth) * 0.5f);
        ImGui::TextColored(ImVec4(0.3f, 0.6f, 1.0f, 1.0f), "CONTROLS");
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Dummy(ImVec2(0.0f, 5.0f));

        float colWidth = 260.0f;
        float startX1 = (600.0f - colWidth * 2.0f) * 0.5f + 20.0f;
        float startX2 = startX1 + colWidth;

        ImGui::SetCursorPosX(startX1);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Move: ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "W, A, S, D");

        ImGui::SameLine(startX2);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Rotate: ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Mouse");

        ImGui::Spacing();

        ImGui::SetCursorPosX(startX1);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Jump: ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Space");

        ImGui::SameLine(startX2);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Kick: ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Left Control");

        ImGui::Dummy(ImVec2(0.0f, 25.0f));

        float buttonWidth = 180.0f;
        float buttonHeight = 40.0f;
        float spacing = 20.0f;
        float totalWidth = buttonWidth * 2 + spacing;
        ImGui::SetCursorPosX((600.0f - totalWidth) * 0.5f);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
        if (ImGui::Button("BACK", ImVec2(buttonWidth, buttonHeight)))
        {
            m_state = MenuState::Main;
        }
        ImGui::PopStyleColor();

        ImGui::SameLine(0.0f, spacing);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.6f, 0.2f, 1.0f));
        if (ImGui::Button("START MATCH", ImVec2(buttonWidth, buttonHeight)))
        {
            Application &app = Application::Get();
            app.loadScene(std::make_unique<DefaultScene>(app.getWhiteTexture(), m_playerCharIdx, m_enemyCharIdx));
        }
        ImGui::PopStyleColor();

        ImGui::End();

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(3);
    }
}
