#include "application.hpp"

#include <glad/glad.h>

#include <iostream>
#include <pmmintrin.h>
#include <xmmintrin.h>

#include "GLFW/glfw3.h"
#include "debug.hpp"
#include "graphics/shader.hpp"
#include "input.hpp"
#include "window.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

Application *Application::s_Instance = nullptr;

Application::Application() : m_isRunning(true)
{
    s_Instance = this;

    m_window = std::make_unique<Window>(1280, 720, "Projekt GK");

    m_inputManager =
        std::make_unique<InputManager>(m_window->getNativeWindow());

    m_inputManager->bindAction("forward", GLFW_KEY_W);
    m_inputManager->bindAction("back", GLFW_KEY_S);
    m_inputManager->bindAction("left", GLFW_KEY_A);
    m_inputManager->bindAction("right", GLFW_KEY_D);
    m_inputManager->bindAction("shoot", GLFW_KEY_LEFT_CONTROL);
    m_inputManager->bindAction("jump", GLFW_KEY_SPACE);
    loadShader("default", "assets/shaders/default.vert",
               "assets/shaders/default.frag");
    loadShader("unlit", "assets/shaders/default.vert",
               "assets/shaders/unlit.frag");
    loadShader("light", "assets/shaders/light.vert",
               "assets/shaders/light.frag");
    loadShader("lineDebug", "assets/shaders/lineDebug.vert",
               "assets/shaders/lineDebug.frag");
    loadShader("skybox", "assets/shaders/skybox.vert",
               "assets/shaders/skybox.frag");
    Debug::init();

    glEnable(GL_DEPTH_TEST);

    glGenTextures(1, &whiteTexture);
    glBindTexture(GL_TEXTURE_2D, whiteTexture);

    unsigned char whitePixel[] = {255, 255, 255, 255};

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 whitePixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // fix for Denormal Floats
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_window->getNativeWindow(), true);

    const char *glsl_version = "#version 450";
    ImGui_ImplOpenGL3_Init(glsl_version);
}

Application::~Application() { glfwTerminate(); }

void Application::run()
{
    Shader *debugShader = getShader("lineDebug");

    double accumulator = 0.0;
    const double fixedDeltaTime = 1.0 / 60.0;

    while (m_isRunning)
    {
        if (m_nextScene)
        {
            m_activeScene = std::move(m_nextScene);
            if (m_activeScene)
            {
                m_activeScene->init();
            }
            m_lastFrameTime = glfwGetTime();
            accumulator = 0.0;
        }

        if (!m_activeScene)
        {
            m_inputManager->update();
            glfwPollEvents();
            continue;
        }

        double currentFrameTime = glfwGetTime();
        double deltaTime = currentFrameTime - m_lastFrameTime;
        m_lastFrameTime = currentFrameTime;

        if (deltaTime > 0.25)
            deltaTime = 0.25;

        accumulator += deltaTime;

        m_inputManager->update();
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClearColor(0.5f, 0.8f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        while (accumulator >= fixedDeltaTime)
        {
            m_activeScene->fixedUpdate(static_cast<float>(fixedDeltaTime));
            accumulator -= fixedDeltaTime;
        }
        m_activeScene->update(static_cast<float>(deltaTime));
        Debug::update(static_cast<float>(deltaTime));
        m_activeScene->draw();

        debugShader->use();
        debugShader->setMat4("view", 1, GL_FALSE,
                             &m_activeScene->getMainViewMatrix()[0][0]);
        debugShader->setMat4("projection", 1, GL_FALSE,
                             &m_activeScene->getMainProjectionMatrix()[0][0]);
        Debug::render(*debugShader);

        m_activeScene->drawUI();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (m_window->ShouldClose())
        {
            close();
        }

        m_window->SwapBuffers();
    }
}

void Application::close() { m_isRunning = false; }

void Application::loadShader(const std::string &name, const char *vertexPath,
                             const char *fragmentPath)
{
    m_shaders[name] = std::make_unique<Shader>(vertexPath, fragmentPath);
}

Shader *Application::getShader(const std::string &name)
{
    auto it = m_shaders.find(name);
    if (it != m_shaders.end())
    {
        return it->second.get();
    }

    std::cerr << "Error: Couldn't find shader with this name: " << name << "\n";
    return nullptr;
}

void Application::loadScene(std::unique_ptr<BaseScene> scene)
{
    if (!m_activeScene)
    {
        m_activeScene = std::move(scene);
        if (m_activeScene)
            m_activeScene->init();
    }
    else
    {
        m_nextScene = std::move(scene);
    }
}
