#pragma once

#include "world/scene.hpp"
#include <memory>

class HeadlessApplication
{
  public:
    HeadlessApplication();
    ~HeadlessApplication();

    void step(float deltaTime);

    static HeadlessApplication &Get() { return *s_Instance; }

    void loadScene(std::unique_ptr<Scene> scene);

  private:
    std::unique_ptr<Scene> m_activeScene;
    static HeadlessApplication *s_Instance;
};
