#include "renderSystem.hpp"

#include "world/components/meshRenderer.hpp"
#include <glm/glm.hpp>
#include <memory>

#include "core/application.hpp"
#include "world/components/meshRenderer.hpp"
#include "world/components/transform.hpp"

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

    lightShader->use();
    for (const std::unique_ptr<Entity> &entity : entities)
    {
        DirectionalLight *light = entity->GetComponent<DirectionalLight>();
        if (!light)
            continue;
        Transform *transform = entity->GetComponent<Transform>();
        if (!transform)
            continue;
        renderDirLight(transform, light, lightShader);
    }

    for (const std::unique_ptr<Entity> &entity : entities)
    {
        MeshRenderer *mesh = entity->GetComponent<MeshRenderer>();
        if (!mesh)
            continue;
        Transform *transform = entity->GetComponent<Transform>();
        if (!transform)
            continue;
        drawEntity(mesh, transform, projection, view);
    }
}

void RenderSystem::drawEntity(MeshRenderer *mesh, Transform *transform,
                              const glm::mat4 &projection,
                              const glm::mat4 &view)
{
    mesh->m_shader->use();

    glm::mat4 model = transform->getModelMatrix();
    glm::mat4 offset = glm::translate(glm::mat4(1.0f), mesh->m_offset);
    model = offset * model;

    mesh->m_shader->setMat4("u_projection", 1, GL_FALSE, &projection[0][0]);
    mesh->m_shader->setMat4("u_view", 1, GL_FALSE, &view[0][0]);
    mesh->m_shader->setMat4("u_model", 1, GL_FALSE, &model[0][0]);

    mesh->m_model->draw(*mesh->m_shader, m_whiteTextureId);
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
