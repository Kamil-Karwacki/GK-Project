#pragma once
#include <GLFW/glfw3.h>
#include <string>

class Window 
{
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    void SwapBuffers();
    bool ShouldClose() const;

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    void setWidth(int width) { m_width = width; }
    void setHeight(int height) { m_height = height; }
    GLFWwindow* getNativeWindow() const { return m_window; }

private:
    GLFWwindow* m_window;
    int m_width, m_height;
    std::string m_title;
};