#pragma once
#include <memory>
#include <vector>

#include "world/components/light.hpp"
#include "world/components/transform.hpp"
#include "world/entity.hpp"

class MeshRenderer;
class Shader;

class RenderSystem
{
  public:
    RenderSystem(unsigned int whiteTextureId) : m_whiteTextureId(whiteTextureId)
    {
    }

    void render(std::vector<std::unique_ptr<Entity>> &entities,
                Entity *cameraEntity);
    void drawEntity(MeshRenderer *mesh, Transform *transform,
                    const glm::mat4 &projection, const glm::mat4 &view);
    void renderDirLight(Transform *transform, DirectionalLight *light,
                        Shader *shader);

    unsigned int m_whiteTextureId;
};
