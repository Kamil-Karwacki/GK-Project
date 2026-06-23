#pragma once
#include "miniaudio.h"
#include "world/baseScene.hpp"
#include <string>

enum class GameState
{
    Playing,
    GoalScored,
    Countdown,
    GameFinished
};

class DefaultScene : public BaseScene
{
  public:
    DefaultScene(unsigned int whiteTextureId, int playerCharIdx = 0,
                 int enemyCharIdx = 0);
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

    GameState m_gameState = GameState::Countdown;
    float m_stateTimer = 3.0f;
    std::string m_goalText = "";

    ma_sound m_matchMusic;
    bool m_isSoundInitialized = false;

  private:
    unsigned int m_skyboxVAO = 0;
    unsigned int m_skyboxVBO = 0;

    int m_playerCharIdx;
    int m_enemyCharIdx;
};
