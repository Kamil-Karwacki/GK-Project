#pragma once
#include "world/baseScene.hpp"

class DefaultScene : public BaseScene
{
public:
    using BaseScene::BaseScene;
    void init() override;
    void update(float deltaTime) override;
    void fixedUpdate(float deltaTime) override;
    void draw() override;
    void drawUI() override;
    void generateTerrain();

    uint16_t m_playerScore = 0;
    uint16_t m_enemyScore = 0;
    float m_matchTimer = 120.0f;

    glm::vec3 m_playerGatePos;
    glm::vec3 m_enemyGatePos;
};
