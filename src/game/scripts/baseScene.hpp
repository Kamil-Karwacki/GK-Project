#pragma once
#include "glm/ext/vector_float2.hpp"
#include "graphics/shader.hpp"
#include "world/scene.hpp"

class BaseScene : public Scene
{
  public:
    using Scene::Scene;
    void init() override;
    void update(float deltaTime) override;
    void fixedUpdate(float deltaTime) override;
    void draw() override;
    void generateTerrain();
    void generatePitch(glm::vec2 pitchSize, glm::vec2 groundAdd,
                       float wallHeight, float bannerLength,
                       Shader *defaultShader);
    void generateGates(glm::vec2 pitchSize, glm::vec3 gateSize,
                       float gateThickness, Shader *defaultShader);

    glm::vec3 m_playerGatePos;
    glm::vec3 m_enemyGatePos;
};
