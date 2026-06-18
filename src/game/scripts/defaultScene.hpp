#pragma once
#include "world/baseScene.hpp"

class DefaultScene : public BaseScene
{
  public:
    using BaseScene::BaseScene;
    ~DefaultScene() override;
    void init() override;
    void update(float deltaTime) override;
    void fixedUpdate(float deltaTime) override;
    void draw() override;
    void drawUI() override;
    void generateTerrain();
    void generatePowerups(Entity *cameraPlayer);

    uint16_t m_playerScore = 0;
    uint16_t m_enemyScore = 0;
    float m_matchTimer = 120.0f;

    glm::vec3 m_playerGatePos;
    glm::vec3 m_enemyGatePos;

    void resetPositions();

    Entity *m_player = nullptr;
    Entity *m_enemy = nullptr;
    Entity *m_ball = nullptr;

  private:
    unsigned int m_skyboxVAO = 0;
    unsigned int m_skyboxVBO = 0;
};
