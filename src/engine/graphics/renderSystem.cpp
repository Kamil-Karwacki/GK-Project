#include "renderSystem.hpp"
#include <glad/glad.h>

#include "world/components/meshRenderer.hpp"
#include <glm/glm.hpp>
#include <memory>

#include "core/application.hpp"
#include "world/components/meshRenderer.hpp"
#include "world/components/transform.hpp"
#include <glm/gtc/matrix_transform.hpp>

RenderSystem::RenderSystem(unsigned int whiteTextureId)
    : m_whiteTextureId(whiteTextureId)
{
    glGenFramebuffers(1, &m_depthMapFBO);
    glGenTextures(1, &m_depthMap);
    glBindTexture(GL_TEXTURE_2D, m_depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 2048, 2048, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           m_depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderSystem::render(std::vector<std::unique_ptr<Entity>> &entities,
                          Entity *cameraEntity)
{
    if (!cameraEntity)
        return;
    Camera *camera = cameraEntity->GetComponent<Camera>();
    Transform *cameraTransform = cameraEntity->GetComponent<Transform>();
    if (!camera || !cameraTransform)
        return;

    glm::mat4 projection = camera->getProjection();
    glm::mat4 view = glm::inverse(cameraTransform->getModelMatrix());

    Application &app = Application::Get();
    Shader *lightShader = app.getShader("default");
    Shader *depthShader = app.getShader("depth");

    glm::vec3 lightDir = glm::vec3(0.0f, -1.0f, 0.0f);
    DirectionalLight *dLight = nullptr;
    Transform *dLightTransform = nullptr;

    for (const std::unique_ptr<Entity> &entity : entities)
    {
        DirectionalLight *light = entity->GetComponent<DirectionalLight>();
        if (light)
        {
            dLight = light;
            dLightTransform = entity->GetComponent<Transform>();
            lightDir = dLightTransform->getFront();
            break;
        }
    }

    glm::mat4 lightProjection =
        glm::ortho(-150.0f, 150.0f, -150.0f, 150.0f, 1.0f, 500.0f);
    glm::mat4 lightView = glm::lookAt(-lightDir * 200.0f, glm::vec3(0.0f),
                                      glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    // Depth map
    glViewport(0, 0, 2048, 2048);
    glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    depthShader->use();
    depthShader->setMat4("u_lightSpaceMatrix", 1, GL_FALSE,
                         &lightSpaceMatrix[0][0]);

    for (const std::unique_ptr<Entity> &entity : entities)
    {
        MeshRenderer *mesh = entity->GetComponent<MeshRenderer>();
        Transform *transform = entity->GetComponent<Transform>();
        if (mesh && transform)
            drawEntity(mesh, transform, lightProjection, lightView,
                       depthShader);
    }

    // Main render
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, app.getWindowWidth(), app.getWindowHeight());

    lightShader->use();
    if (dLight)
        renderDirLight(dLightTransform, dLight, lightShader);

    lightShader->setMat4("u_lightSpaceMatrix", 1, GL_FALSE,
                         &lightSpaceMatrix[0][0]);
    lightShader->setInt("u_shadowMap", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_depthMap);

    for (const std::unique_ptr<Entity> &entity : entities)
    {
        MeshRenderer *mesh = entity->GetComponent<MeshRenderer>();
        Transform *transform = entity->GetComponent<Transform>();
        if (mesh && transform)
            drawEntity(mesh, transform, projection, view, nullptr);
    }
}

void RenderSystem::drawEntity(MeshRenderer *mesh, Transform *transform,
                              const glm::mat4 &projection,
                              const glm::mat4 &view, Shader *shaderOverride)
{
    Shader *activeShader = shaderOverride ? shaderOverride : mesh->m_shader;
    activeShader->use();

    glm::mat4 model = transform->getModelMatrix();
    glm::mat4 offset = glm::translate(glm::mat4(1.0f), mesh->m_offset);
    model = offset * model;

    activeShader->setMat4("u_projection", 1, GL_FALSE, &projection[0][0]);
    activeShader->setMat4("u_view", 1, GL_FALSE, &view[0][0]);
    activeShader->setMat4("u_model", 1, GL_FALSE, &model[0][0]);

    if (!shaderOverride)
    {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_depthMap);
        activeShader->setInt("u_shadowMap", 2);
        glActiveTexture(GL_TEXTURE0);
    }

    mesh->m_model->draw(*activeShader, m_whiteTextureId);
}

void RenderSystem::renderDirLight(Transform *transform, DirectionalLight *light,
                                  Shader *shader)
{
    glm::vec3 dir = transform->getFront();
    shader->setVec3("u_dirLight.direction", -dir);
    shader->setVec3("u_dirLight.color", light->m_color);
    shader->setFloat("u_dirLight.ambient", light->m_ambient);
    shader->setFloat("u_dirLight.diffuse", light->m_diffuse);
    shader->setFloat("u_dirLight.specular", light->m_specular);
}
