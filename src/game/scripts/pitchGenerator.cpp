#include "pitchGenerator.hpp"
#include "scripts/gateTrigger.hpp"
#include "world/components/collider.hpp"
#include "world/components/transform.hpp"
#include "world/scene.hpp"
#include <memory>

#ifndef HEADLESS_MODE
#include "graphics/shader.hpp"
#include "stb_image.h"
#include "world/components/meshRenderer.hpp"
#include <cmath>
#include <glad/glad.h>
#include <iostream>

static unsigned int loadTexture(const std::string &path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char *data =
        stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
    if (data)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cerr << "Failed to load texture: " << path << "\n";
        textureID = 0;
    }
    stbi_image_free(data);
    return textureID;
}

static std::shared_ptr<Model> createQuadModel(float width, float height,
                                              float tilingX, float tilingY,
                                              const glm::vec3 &color,
                                              unsigned int textureID)
{
    std::vector<Vertex> vertices = {{{-width / 2.0f, -height / 2.0f, 0.0f},
                                     {0.0f, 0.0f, 1.0f},
                                     {0.0f, 0.0f},
                                     color},
                                    {{width / 2.0f, -height / 2.0f, 0.0f},
                                     {0.0f, 0.0f, 1.0f},
                                     {tilingX, 0.0f},
                                     color},
                                    {{width / 2.0f, height / 2.0f, 0.0f},
                                     {0.0f, 0.0f, 1.0f},
                                     {tilingX, tilingY},
                                     color},
                                    {{-width / 2.0f, height / 2.0f, 0.0f},
                                     {0.0f, 0.0f, 1.0f},
                                     {0.0f, tilingY},
                                     color}};
    std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0};
    std::vector<Texture> textures;
    if (textureID != 0)
    {
        Texture tex;
        tex.id = textureID;
        tex.type = "texture_diffuse";
        textures.push_back(tex);
    }
    return std::make_shared<Model>(Mesh(vertices, indices, textures));
}

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

#ifndef HEADLESS_MODE
    static unsigned int netTextureID = 0;
    if (netTextureID == 0 && defaultShader)
    {
        netTextureID = loadTexture("assets/textures/net.png");
    }

    if (defaultShader && netTextureID != 0)
    {
        glm::vec3 netColor(1.0f, 1.0f, 1.0f);

        float sideWidth = config.gateSize.z;
        float sideHeight = config.gateSize.y;
        auto sideNetModel =
            createQuadModel(sideWidth, sideHeight, sideWidth / 2.0f,
                            sideHeight / 2.0f, netColor, netTextureID);

        float backWidth = config.gateSize.x;
        float backHeight = config.gateSize.y;
        auto backNetModel =
            createQuadModel(backWidth, backHeight, backWidth / 2.0f,
                            backHeight / 2.0f, netColor, netTextureID);

        float topWidth = config.gateSize.x;
        float topDepth = config.gateSize.z;
        auto topNetModel =
            createQuadModel(topWidth, topDepth, topWidth / 2.0f,
                            topDepth / 2.0f, netColor, netTextureID);

        for (int j = 1; j > -2; j -= 2)
        {
            float gateCenterZ =
                j * config.pitchSize.x / 2.0f - (config.gateSize.z / 2.0f) * j;
            float backNetZ = j * config.pitchSize.x / 2.0f - 0.5f * j;

            // 1. Left side net
            {
                auto entSideL = std::make_unique<Entity>(scene);
                entSideL->AddComponent<Transform>(
                    offset + glm::vec3(-config.gateSize.x / 2.0f,
                                       config.gateSize.y / 2.0f, gateCenterZ),
                    glm::vec3(0.0f, glm::radians(90.0f), 0.0f),
                    glm::vec3(1.0f));
                entSideL->AddComponent<MeshRenderer>(sideNetModel,
                                                     defaultShader);
                targetEntities.push_back(std::move(entSideL));
            }

            // 2. Right side net
            {
                auto entSideR = std::make_unique<Entity>(scene);
                entSideR->AddComponent<Transform>(
                    offset + glm::vec3(config.gateSize.x / 2.0f,
                                       config.gateSize.y / 2.0f, gateCenterZ),
                    glm::vec3(0.0f, glm::radians(90.0f), 0.0f),
                    glm::vec3(1.0f));
                entSideR->AddComponent<MeshRenderer>(sideNetModel,
                                                     defaultShader);
                targetEntities.push_back(std::move(entSideR));
            }

            // 3. Back net
            {
                auto entBack = std::make_unique<Entity>(scene);
                entBack->AddComponent<Transform>(
                    offset +
                        glm::vec3(0.0f, config.gateSize.y / 2.0f, backNetZ),
                    glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
                entBack->AddComponent<MeshRenderer>(backNetModel,
                                                    defaultShader);
                targetEntities.push_back(std::move(entBack));
            }

            // 4. Top net
            {
                auto entTop = std::make_unique<Entity>(scene);
                entTop->AddComponent<Transform>(
                    offset + glm::vec3(0.0f, config.gateSize.y, gateCenterZ),
                    glm::vec3(glm::radians(90.0f), 0.0f, 0.0f),
                    glm::vec3(1.0f));
                entTop->AddComponent<MeshRenderer>(topNetModel, defaultShader);
                targetEntities.push_back(std::move(entTop));
            }
        }
    }
#endif

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
