#include "headlessApplication.hpp"

#include <pmmintrin.h>
#include <xmmintrin.h>

#include "world/entity.hpp"

HeadlessApplication *HeadlessApplication::s_Instance = nullptr;

HeadlessApplication::HeadlessApplication()
{
    s_Instance = this;

    // fix for Denormal Floats
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
}

HeadlessApplication::~HeadlessApplication() = default;

void HeadlessApplication::step(float deltaTime)
{
    m_activeScene->update(deltaTime);
    m_activeScene->fixedUpdate(deltaTime);
}

void HeadlessApplication::loadScene(std::unique_ptr<Scene> scene)
{
    m_activeScene = std::move(scene);
    if (m_activeScene)
        m_activeScene->init();
}
