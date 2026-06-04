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

    std::vector<MatchArena> m_arenas;
};
