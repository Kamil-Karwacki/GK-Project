#include "input.hpp"
#include <iostream>

InputManager::InputManager(GLFWwindow *window)
{
    std::fill(std::begin(m_currentKeys), std::end(m_currentKeys), false);
    std::fill(std::begin(m_previousKeys), std::end(m_previousKeys), false);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallback);

    for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; ++i)
    {
        m_currentMouse[i] = false;
        m_previousMouse[i] = false;
    }

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
}

void InputManager::update()
{
    std::copy(std::begin(m_currentKeys), std::end(m_currentKeys),
              std::begin(m_previousKeys));
    for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; ++i)
    {
        m_previousMouse[i] = m_currentMouse[i];
    }
    m_mouseDeltaX = 0.0;
    m_mouseDeltaY = 0.0;
}

bool InputManager::isKeyDown(int key) const
{
    if (key < 0 || key > GLFW_KEY_LAST)
        return false;
    return m_currentKeys[key];
}

bool InputManager::isKeyPressed(int key) const
{
    if (key < 0 || key > GLFW_KEY_LAST)
        return false;
    return m_currentKeys[key] && !m_previousKeys[key];
}

bool InputManager::isKeyReleased(int key) const
{
    if (key < 0 || key > GLFW_KEY_LAST)
        return false;
    return !m_currentKeys[key] && m_previousKeys[key];
}

void InputManager::bindAction(const std::string &actionName, int key)
{
    m_actionBindings[actionName] = key;
}

bool InputManager::isActionPressed(const std::string &actionName)
{
    auto it = m_actionBindings.find(actionName);
    if (it != m_actionBindings.end())
    {
        return isKeyPressed(it->second);
    }
    return false;
}

bool InputManager::isActionHeld(const std::string &actionName)
{
    auto it = m_actionBindings.find(actionName);
    if (it != m_actionBindings.end())
    {
        return isKeyDown(it->second);
    }
    return false;
}

bool InputManager::isActionReleased(const std::string &actionName)
{
    auto it = m_actionBindings.find(actionName);
    if (it != m_actionBindings.end())
    {
        return isKeyReleased(it->second);
    }
    return false;
}

void InputManager::keyCallback(GLFWwindow *window, int key, int scancode,
                               int action, int mods)
{
    if (key < 0 || key > GLFW_KEY_LAST)
        return;

    InputManager *input =
        static_cast<InputManager *>(glfwGetWindowUserPointer(window));

    if (input)
    {
        if (action == GLFW_PRESS)
        {
            input->m_currentKeys[key] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            input->m_currentKeys[key] = false;
        }
    }
}

void InputManager::mouseButtonCallback(GLFWwindow *window, int button,
                                       int action, int mods)
{
    InputManager *manager =
        static_cast<InputManager *>(glfwGetWindowUserPointer(window));
    if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST)
    {
        if (action == GLFW_PRESS)
            manager->m_currentMouse[button] = true;
        else if (action == GLFW_RELEASE)
            manager->m_currentMouse[button] = false;
    }
}

void InputManager::cursorPosCallback(GLFWwindow *window, double xpos,
                                     double ypos)
{
    InputManager *manager =
        static_cast<InputManager *>(glfwGetWindowUserPointer(window));

    if (manager->m_firstMouse)
    {
        manager->m_mouseX = xpos;
        manager->m_mouseY = ypos;
        manager->m_firstMouse = false;
    }

    manager->m_mouseDeltaX = xpos - manager->m_mouseX;
    manager->m_mouseDeltaY = manager->m_mouseY - ypos;

    manager->m_mouseX = xpos;
    manager->m_mouseY = ypos;
}
