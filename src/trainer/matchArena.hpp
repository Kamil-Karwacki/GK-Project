#pragma once
#include "physics/physicsSystem.hpp"

struct MatchArena
{
    std::unique_ptr<PhysicsSystem> m_physics;
    std::vector<std::unique_ptr<Entity>> m_entities;

    Entity *m_playerA = nullptr;
    Entity *m_playerB = nullptr;
    Entity *m_ball = nullptr;

    float m_fitnessA = 0.0f;
    float m_fitnessB = 0.0f;
    uint32_t m_arenaID = 0;
    bool m_needsReset = false;
    uint32_t m_framesSinceLastReset = 0;
    glm::vec3 m_arenaOffset;

    glm::vec3 m_gateAPos;
    glm::vec3 m_gateBPos;

    float m_prevBallToGateA;
    float m_prevBallToGateB;

    MatchArena(int id);

    Entity &createEntity(Scene *scene);

    void resetPositions();

    void fixedUpdate(float deltaTime);
};
