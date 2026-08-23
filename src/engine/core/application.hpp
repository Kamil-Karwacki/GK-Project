#pragma once

#include <memory>

#include "graphics/shader.hpp"
#include "input.hpp"
#include "miniaudio.h"
#include "window.hpp"
#include "world/baseScene.hpp"

class Application
{
  public:
    Application();
    ~Application();
    Application(int argc, char *argv[]);

    void run();
    void close();

    static Application &Get() { return *s_Instance; }

    InputManager &GetInput() { return *m_inputManager; }

    int getWindowWidth() { return m_window->getWidth(); }
    int getWindowHeight() { return m_window->getHeight(); }

    void loadShader(const std::string &name, const char *vertexPath,
                    const char *fragmentPath);
    Shader *getShader(const std::string &name);
    unsigned int getWhiteTexture() { return whiteTexture; }

    void loadScene(std::unique_ptr<BaseScene> scene);

    std::unique_ptr<Window> m_window;
    std::unique_ptr<BaseScene> m_activeScene;

    ma_engine m_soundEngine;

    std::vector<std::string_view> m_args;

  private:
    std::unique_ptr<InputManager> m_inputManager;

    double m_lastFrameTime = 0.0f;

    bool m_isRunning;
    static Application *s_Instance;

    unsigned int whiteTexture;
    std::unordered_map<std::string, std::unique_ptr<Shader>> m_shaders;
    std::unique_ptr<BaseScene> m_nextScene;
};
