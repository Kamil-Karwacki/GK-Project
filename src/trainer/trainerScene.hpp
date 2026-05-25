#pragma once
#include "glm/ext/vector_float2.hpp"
#include "graphics/shader.hpp"
#include "physics/physicsSystem.hpp"
#include "world/components/transform.hpp"
#include "world/scene.hpp"

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

    MatchArena(int id) : m_arenaID(id)
    {
        m_physics = std::make_unique<PhysicsSystem>();
        float offsetX = (m_arenaID % 10) * 150.0f;
        float offsetZ = (static_cast<float>(m_arenaID) / 10) * 200.0f;
        m_arenaOffset = {offsetX, 0, offsetZ};
    }

    Entity &createEntity(Scene *scene)
    {
        auto entity = std::make_unique<Entity>(scene);
        Entity *rawPtr = entity.get();
        m_entities.push_back(std::move(entity));
        return *rawPtr;
    }

    void resetPositions()
    {
        m_playerA->GetComponent<Transform>()->setPosition(m_arenaOffset +
                                                          glm::vec3(0, 2, 50));
        m_playerA->GetComponent<Rigidbody>()->m_velocity = glm::vec3(0);
        m_playerA->GetComponent<Rigidbody>()->m_angularVelocity = glm::vec3(0);

        m_playerB->GetComponent<Transform>()->setPosition(m_arenaOffset +
                                                          glm::vec3(0, 2, -50));
        m_playerB->GetComponent<Rigidbody>()->m_velocity = glm::vec3(0);
        m_playerB->GetComponent<Rigidbody>()->m_angularVelocity = glm::vec3(0);

        m_ball->GetComponent<Transform>()->setPosition(m_arenaOffset +
                                                       glm::vec3(0, 5, 0));
        m_ball->GetComponent<Rigidbody>()->m_velocity = glm::vec3(0);

        m_needsReset = false;
        m_framesSinceLastReset = 0;
        glm::vec3 ballStart = m_arenaOffset + glm::vec3(0, 5, 0);
        m_prevBallToGateA = glm::distance(ballStart, m_gateAPos);
        m_prevBallToGateB = glm::distance(ballStart, m_gateBPos);
    }

    void fixedUpdate(float deltaTime)
    {
        m_physics->update(m_entities, deltaTime);
        m_physics->generateContacts(m_entities);
        m_physics->resolveContacts(deltaTime);
    }
};

class TrainerScene : public Scene
{
  public:
    using Scene::Scene;
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
