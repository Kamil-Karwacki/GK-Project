#pragma once
#include "matchArena.hpp"
#include "world/scene.hpp"

class HeadlessTrainerScene : public Scene
{
  public:
    using Scene::Scene;
    void init() override;
    void update(float deltaTime) override;
    void fixedUpdate(float deltaTime) override;
    void generateArena(MatchArena &arena);
    void generatePitch(MatchArena &arena, glm::vec2 pitchSize,
                       glm::vec2 groundAdd, float wallHeight,
                       float bannerLength);

    void generateGates(MatchArena &arena, glm::vec2 pitchSize,
                       glm::vec3 gateSize, float gateThickness);

    std::vector<MatchArena> m_arenas;
};
