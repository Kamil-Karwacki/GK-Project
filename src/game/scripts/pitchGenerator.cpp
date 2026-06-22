#include "pitchGenerator.hpp"
#include "scripts/gateTrigger.hpp"
#include "world/components/collider.hpp"
#include "world/components/transform.hpp"
#include "world/scene.hpp"
#include <memory>

#ifndef HEADLESS_MODE
#include "graphics/shader.hpp"
#include "world/components/meshRenderer.hpp"
#include <cmath>

static void addBoxToMesh(std::vector<Vertex> &vertices,
                         std::vector<unsigned int> &indices,
                         const glm::vec3 &position, const glm::vec3 &size,
                         const glm::vec3 &color)
{
    Mesh boxMesh = Mesh::createBox(size, color);
    unsigned int indexOffset = vertices.size();
    for (const auto &v : boxMesh.m_vertices)
    {
        Vertex newV = v;
        newV.position += position;
        vertices.push_back(newV);
    }
    for (unsigned int idx : boxMesh.m_indices)
    {
        indices.push_back(idx + indexOffset);
    }
}

static Mesh generatePitchLinesMesh(const glm::vec2 &pitchSize, float lineWidth,
                                   const glm::vec3 &color)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float pLength = pitchSize.x;
    float pWidth = pitchSize.y;

    float margin = 7.0f;
    float l = pLength - margin * 2.0f;
    float w = pWidth - margin * 2.0f;

    addBoxToMesh(vertices, indices, glm::vec3(0, 0, -l / 2.0f),
                 glm::vec3(w + lineWidth, lineWidth, lineWidth), color);
    addBoxToMesh(vertices, indices, glm::vec3(0, 0, l / 2.0f),
                 glm::vec3(w + lineWidth, lineWidth, lineWidth), color);
    addBoxToMesh(vertices, indices, glm::vec3(-w / 2.0f, 0, 0),
                 glm::vec3(lineWidth, lineWidth, l - lineWidth), color);
    addBoxToMesh(vertices, indices, glm::vec3(w / 2.0f, 0, 0),
                 glm::vec3(lineWidth, lineWidth, l - lineWidth), color);

    addBoxToMesh(vertices, indices, glm::vec3(0, 0, 0),
                 glm::vec3(w, lineWidth, lineWidth), color);

    float innerRadius = 9.15f;
    float outerRadius = innerRadius + lineWidth;
    int segments = 64;
    float halfHeight = lineWidth * 0.5f;
    glm::vec3 normalUp(0, 1, 0);

    for (int i = 0; i < segments; ++i)
    {
        float angle1 = (float)i / segments * 2.0f * 3.14159265359f;
        float angle2 = (float)(i + 1) / segments * 2.0f * 3.14159265359f;

        glm::vec3 p1Inner(std::cos(angle1) * innerRadius, 0.0f,
                          std::sin(angle1) * innerRadius);
        glm::vec3 p1Outer(std::cos(angle1) * outerRadius, 0.0f,
                          std::sin(angle1) * outerRadius);
        glm::vec3 p2Inner(std::cos(angle2) * innerRadius, 0.0f,
                          std::sin(angle2) * innerRadius);
        glm::vec3 p2Outer(std::cos(angle2) * outerRadius, 0.0f,
                          std::sin(angle2) * outerRadius);

        unsigned int topIdx = vertices.size();
        vertices.push_back(
            {p1Inner + glm::vec3(0, halfHeight, 0), normalUp, {0, 0}, color});
        vertices.push_back(
            {p1Outer + glm::vec3(0, halfHeight, 0), normalUp, {0, 0}, color});
        vertices.push_back(
            {p2Outer + glm::vec3(0, halfHeight, 0), normalUp, {0, 0}, color});
        vertices.push_back(
            {p2Inner + glm::vec3(0, halfHeight, 0), normalUp, {0, 0}, color});

        indices.push_back(topIdx);
        indices.push_back(topIdx + 1);
        indices.push_back(topIdx + 2);
        indices.push_back(topIdx + 2);
        indices.push_back(topIdx + 3);
        indices.push_back(topIdx);
    }

    return Mesh(vertices, indices, std::vector<Texture>());
}
#endif

namespace PitchGenerator
{
void generatePitch(std::vector<std::unique_ptr<Entity>> &targetEntities,
                   Scene *scene, const glm::vec3 &offset, Shader *defaultShader,
                   const PitchConfig &config)
{
    auto groundEnt = std::make_unique<Entity>(scene);
    Entity &ground = *groundEnt;
    targetEntities.push_back(std::move(groundEnt));

    ground.AddComponent<Transform>(offset);

#ifndef HEADLESS_MODE
    if (defaultShader)
    {
        ground.AddComponent<MeshRenderer>(
            std::make_shared<Model>(Mesh::createBox(
                glm::vec3(config.pitchSize.y, 0.5f, config.pitchSize.x),
                glm::vec3(0.2f, 0.5f, 0.2f))),
            defaultShader);

        auto linesEnt = std::make_unique<Entity>(scene);
        linesEnt->AddComponent<Transform>(offset +
                                          glm::vec3(0.0f, 0.26f, 0.0f));
        linesEnt->AddComponent<MeshRenderer>(
            std::make_shared<Model>(generatePitchLinesMesh(
                config.pitchSize, 0.3f, glm::vec3(1.0f, 1.0f, 1.0f))),
            defaultShader);
        targetEntities.push_back(std::move(linesEnt));
    }
#endif

    Collider &groundCol = ground.AddComponentAs<Collider, HalfspaceCollider>(
        glm::vec3(0.0f, 1.0f, 0.0f), 0.0f);
    groundCol.m_layer = CAT_GROUND;
    groundCol.m_mask = CAT_PLAYER | CAT_ENEMY | CAT_BALL;
    groundCol.m_restitution = 0.4f;

#ifndef HEADLESS_MODE
    std::shared_ptr<Model> bannerModelA = nullptr;
    if (defaultShader)
    {
        bannerModelA = std::make_shared<Model>("assets/models/baner1.obj");
    }
#endif

    glm::vec2 bannerCount = glm::round(config.pitchSize / config.bannerLength);
    glm::vec2 bannerLengths = config.pitchSize / bannerCount;
    glm::vec2 bannerScale = bannerLengths / config.bannerLength;

    for (size_t i = 0; i < bannerCount.y; i++)
    {
        auto entA = std::make_unique<Entity>(scene);
        Entity &wallA = *entA;
        targetEntities.push_back(std::move(entA));

        Transform &transA = wallA.AddComponent<Transform>();
        transA.setPosition(offset + glm::vec3((config.pitchSize.y / 2.0f) -
                                                  (bannerLengths.y / 2.0f) -
                                                  bannerLengths.y * i,
                                              0.0f, config.pitchSize.x / 2.0f));
        transA.setScale(glm::vec3(bannerScale.y, 1.0f, 1.0f));
        transA.setRotation(
            glm::vec3(glm::radians(90.0f), glm::radians(180.0f), 0));

#ifndef HEADLESS_MODE
        if (defaultShader)
        {
            wallA.AddComponent<MeshRenderer>(
                bannerModelA, defaultShader,
                glm::vec3(0.0f, config.wallHeight / 2.0f, 0.0f));
        }
#endif

        if (i == 0)
        {
            glm::vec3 normal(0.0f, 0.0f, -1.0f);
            float d = (-config.pitchSize.x / 2.0f) + glm::dot(normal, offset);
            auto &wallCol =
                wallA.AddComponentAs<Collider, HalfspaceCollider>(normal, d);
            wallCol.m_layer = CAT_GROUND;
            wallCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_ENEMY;
        }

        auto entB = std::make_unique<Entity>(scene);
        Entity &wallB = *entB;
        targetEntities.push_back(std::move(entB));

        Transform &transB = wallB.AddComponent<Transform>();
        transB.setPosition(offset + glm::vec3((config.pitchSize.y / 2.0f) -
                                                  (bannerLengths.y / 2.0f) -
                                                  bannerLengths.y * i,
                                              0.0f,
                                              -config.pitchSize.x / 2.0f));
        transB.setScale(glm::vec3(bannerScale.y, 1.0f, 1.0f));
        transB.setRotation(
            glm::vec3(glm::radians(90.0f), glm::radians(180.0f), 0));

#ifndef HEADLESS_MODE
        if (defaultShader)
        {
            wallB.AddComponent<MeshRenderer>(
                bannerModelA, defaultShader,
                glm::vec3(0.0f, config.wallHeight / 2.0f, 0.0f));
        }
#endif

        if (i == 0)
        {
            glm::vec3 normal(0.0f, 0.0f, 1.0f);
            float d = (-config.pitchSize.x / 2.0f) + glm::dot(normal, offset);
            HalfspaceCollider &wallCol =
                wallB.AddComponentAs<Collider, HalfspaceCollider>(normal, d);
            wallCol.m_layer = CAT_GROUND;
            wallCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_ENEMY;
        }
    }

    for (size_t i = 0; i < bannerCount.x; i++)
    {
        auto entA = std::make_unique<Entity>(scene);
        Entity &wallA = *entA;
        targetEntities.push_back(std::move(entA));

        Transform &transA = wallA.AddComponent<Transform>();
        transA.setPosition(offset + glm::vec3(config.pitchSize.y / 2.0f, 0.0f,
                                              (-config.pitchSize.x / 2.0f) +
                                                  (bannerLengths.x / 2.0f) +
                                                  bannerLengths.x * i));
        transA.setScale(glm::vec3(bannerScale.x, 1.0f, 1.0f));
        transA.setRotation(
            glm::vec3(glm::radians(90.0f), glm::radians(270.0f), 0));

#ifndef HEADLESS_MODE
        if (defaultShader)
        {
            wallA.AddComponent<MeshRenderer>(
                bannerModelA, defaultShader,
                glm::vec3(0.0f, config.wallHeight / 2.0f, 0.0f));
        }
#endif

        if (i == 0)
        {
            glm::vec3 normal(-1.0f, 0.0f, 0.0f);
            float d = (-config.pitchSize.y / 2.0f) + glm::dot(normal, offset);
            HalfspaceCollider &wallCol =
                wallA.AddComponentAs<Collider, HalfspaceCollider>(normal, d);
            wallCol.m_layer = CAT_GROUND;
            wallCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_ENEMY;
        }

        auto entB = std::make_unique<Entity>(scene);
        Entity &wallB = *entB;
        targetEntities.push_back(std::move(entB));

        Transform &transB = wallB.AddComponent<Transform>();
        transB.setPosition(offset + glm::vec3(-config.pitchSize.y / 2.0f, 0.0f,
                                              (-config.pitchSize.x / 2.0f) +
                                                  (bannerLengths.x / 2.0f) +
                                                  bannerLengths.x * i));
        transB.setScale(glm::vec3(bannerScale.x, 1.0f, 1.0f));
        transB.setRotation(
            glm::vec3(glm::radians(90.0f), glm::radians(90.0f), 0));

#ifndef HEADLESS_MODE
        if (defaultShader)
        {
            wallB.AddComponent<MeshRenderer>(
                bannerModelA, defaultShader,
                glm::vec3(0.0f, config.wallHeight / 2.0f, 0.0f));
        }
#endif

        if (i == 0)
        {
            glm::vec3 normal(1.0f, 0.0f, 0.0f);
            float d = (-config.pitchSize.y / 2.0f) + glm::dot(normal, offset);
            HalfspaceCollider &wallCol =
                wallB.AddComponentAs<Collider, HalfspaceCollider>(normal, d);
            wallCol.m_layer = CAT_GROUND;
            wallCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_ENEMY;
        }
    }
}

GeneratedGatesInfo
generateGates(std::vector<std::unique_ptr<Entity>> &targetEntities,
              Scene *scene, const glm::vec3 &offset, Shader *defaultShader,
              std::function<void()> onGoalScoredA,
              std::function<void()> onGoalScoredB, const PitchConfig &config)
{
    for (int j = 1; j > -2; j -= 2)
    {
        for (int i = 1; i > -2; i -= 2)
        {
            auto entBar = std::make_unique<Entity>(scene);
            Entity &bar = *entBar;
            targetEntities.push_back(std::move(entBar));

            Transform &trans = bar.AddComponent<Transform>();
            trans.setPosition(offset + glm::vec3(i * config.gateSize.x / 2.0f,
                                                 config.gateSize.y / 2.0f,
                                                 j * config.pitchSize.x / 2.0f -
                                                     config.gateSize.z * j));

#ifndef HEADLESS_MODE
            if (defaultShader)
            {
                bar.AddComponent<MeshRenderer>(
                    std::make_shared<Model>(Mesh::createBox(
                        glm::vec3(config.gateThickness, config.gateSize.y,
                                  config.gateThickness),
                        glm::vec3(0.8f, 0.8f, 0.8f))),
                    defaultShader);
            }
#endif

            BoxCollider &boxCol = bar.AddComponentAs<Collider, BoxCollider>(
                glm::vec3(config.gateThickness / 2.0f, config.gateSize.y / 2.0f,
                          config.gateSize.z / 2.0f));
            boxCol.m_offset = glm::translate(
                boxCol.m_offset, glm::vec3(0, 0, j * config.gateSize.z / 2));
            boxCol.m_layer = CAT_GROUND;
            boxCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_ENEMY;
        }
    }

    float ballRadius = 2.5f;

    if (onGoalScoredA)
    {
        auto entA = std::make_unique<Entity>(scene);
        Entity &goalTriggerA = *entA;
        targetEntities.push_back(std::move(entA));

        goalTriggerA.AddComponent<Transform>(
            offset + glm::vec3{0.0f, config.gateSize.y / 2.0f,
                               config.pitchSize.x / 2.0f -
                                   config.gateSize.z / 2.0f + ballRadius * 2});
        goalTriggerA.AddComponentAs<Collider, BoxCollider>(
            glm::vec3{config.gateSize.x * 0.9f / 2.0f, config.gateSize.y / 2.0f,
                      config.gateSize.z / 2.0f},
            glm::mat4(1.0f), true);
        goalTriggerA.AddComponent<GateTrigger>(onGoalScoredA);
    }

    if (onGoalScoredB)
    {
        auto entB = std::make_unique<Entity>(scene);
        Entity &goalTriggerB = *entB;
        targetEntities.push_back(std::move(entB));

        goalTriggerB.AddComponent<Transform>(
            offset + glm::vec3{0.0f, config.gateSize.y / 2.0f,
                               -config.pitchSize.x / 2.0f +
                                   config.gateSize.z / 2.0f - ballRadius * 2});
        goalTriggerB.AddComponentAs<Collider, BoxCollider>(
            glm::vec3{config.gateSize.x * 0.9f / 2.0f, config.gateSize.y / 2.0f,
                      config.gateSize.z / 2.0f},
            glm::mat4(1.0f), true);
        goalTriggerB.AddComponent<GateTrigger>(onGoalScoredB);
    }

    for (int i = 1; i > -2; i -= 2)
    {
        auto entTop = std::make_unique<Entity>(scene);
        Entity &topBar = *entTop;
        targetEntities.push_back(std::move(entTop));

        Transform &topTrans = topBar.AddComponent<Transform>();
        topTrans.setPosition(offset + glm::vec3(0.0f, config.gateSize.y,
                                                i * config.pitchSize.x / 2.0f -
                                                    config.gateSize.z * i));
        topTrans.setRotation(glm::vec3(0.0f, 0.0f, glm::radians(90.0f)));

#ifndef HEADLESS_MODE
        if (defaultShader)
        {
            topBar.AddComponent<MeshRenderer>(
                std::make_shared<Model>(Mesh::createBox(
                    glm::vec3(config.gateThickness,
                              config.gateSize.x + config.gateThickness,
                              config.gateThickness),
                    glm::vec3(0.8f, 0.8f, 0.8f))),
                defaultShader);
        }
#endif

        BoxCollider &boxCol = topBar.AddComponentAs<Collider, BoxCollider>(
            glm::vec3(config.gateThickness / 2.0f, config.gateSize.x / 2.0f,
                      config.gateSize.z / 2.0f));
        boxCol.m_offset = glm::translate(
            boxCol.m_offset, glm::vec3(0, 0, i * config.gateSize.z / 2));
        boxCol.m_layer = CAT_GROUND;
        boxCol.m_mask = CAT_BALL | CAT_PLAYER | CAT_ENEMY;
    }

    GeneratedGatesInfo info;
    info.gateAPos =
        offset + glm::vec3(0.0f, config.gateSize.y / 2.0f,
                           config.pitchSize.x / 2.0f - config.gateSize.z);
    info.gateBPos =
        offset + glm::vec3(0.0f, config.gateSize.y / 2.0f,
                           -config.pitchSize.x / 2.0f + config.gateSize.z);
    return info;
}
} // namespace PitchGenerator
