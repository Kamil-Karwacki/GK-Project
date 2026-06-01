#pragma once
#include "glm/ext/vector_float2.hpp"
#include "graphics/shader.hpp"
#include "matchArena.hpp"
#include "world/baseScene.hpp"
#include "world/scene.hpp"

class TrainerScene : public BaseScene
{
  public:
    using BaseScene::BaseScene;
    void init() override;
    void update(float deltaTime) override;
    void fixedUpdate(float deltaTime) override;
    void draw() override;
    void generateArena(MatchArena &arena);
    void generatePitch(MatchArena &arena, glm::vec2 pitchSize,
                       glm::vec2 groundAdd, float wallHeight,
                       float bannerLength, Shader *defaultShader);

    void generateGates(MatchArena &arena, glm::vec2 pitchSize,
                       glm::vec3 gateSize, float gateThickness,
                       Shader *defaultShader);

    std::vector<MatchArena> m_arenas;
};
