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
    void generateTerrain();

    glm::vec3 m_playerGatePos;
    glm::vec3 m_enemyGatePos;
};
