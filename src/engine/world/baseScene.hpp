#pragma once
#include <memory>
#include <vector>

#include "scene.hpp"
#include "world/components/camera.hpp"

class Entity;
class Behaviour;
class RenderSystem;
class PhysicsSystem;

class BaseScene : public Scene
{
  public:
    BaseScene(unsigned int whiteTextureId);
    ~BaseScene() override;

    void init() override;
    void update(float deltaTime) override;
    void fixedUpdate(float deltaTime) override;
    void draw() override;

    glm::mat4 getMainViewMatrix() const;
    glm::mat4 getMainProjectionMatrix() const;
    void setMainCamera(Camera *camera) { m_mainCamera = camera; }

  protected:
    std::unique_ptr<RenderSystem> m_renderSystem;
    Camera *m_mainCamera = nullptr;
};
