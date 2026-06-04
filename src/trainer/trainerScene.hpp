#pragma once
#include "matchArena.hpp"
#include "world/baseScene.hpp"

class TrainerScene : public BaseScene
{
public:
    using BaseScene::BaseScene;
    void init() override;
    void update(float deltaTime) override;
    void fixedUpdate(float deltaTime) override;
    void draw() override;
    void generateArena(MatchArena &arena);

    std::vector<MatchArena> m_arenas;
};
