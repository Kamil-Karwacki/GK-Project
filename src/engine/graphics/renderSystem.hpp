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
    RenderSystem(unsigned int whiteTextureId);

    void render(std::vector<std::unique_ptr<Entity>> &entities,
                Entity *cameraEntity);
    void drawEntity(MeshRenderer *mesh, Transform *transform,
                    const glm::mat4 &projection, const glm::mat4 &view,
                    Shader *shaderOverride = nullptr);
    void renderDirLight(Transform *transform, DirectionalLight *light,
                        Shader *shader);

    unsigned int m_whiteTextureId;
    unsigned int m_depthMapFBO = 0;
    unsigned int m_depthMap = 0;
};
