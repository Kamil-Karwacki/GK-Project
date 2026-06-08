#pragma once
#include "world/baseScene.hpp"

class Entity;

class MenuScene : public BaseScene
{
public:
    using BaseScene::BaseScene;
    ~MenuScene() override;

    void init() override;
    void update(float deltaTime) override;
    void draw() override;
    void drawUI() override;

private:
    unsigned int m_skyboxVAO = 0;
    unsigned int m_skyboxVBO = 0;
    float m_cameraRotationAngle = 0.0f;
    Entity* m_cameraEntity = nullptr;
};
