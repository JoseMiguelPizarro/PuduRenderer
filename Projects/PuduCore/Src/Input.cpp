//
// Created by Administrator on 6/18/2025.
//

#include "Input.h"


namespace Pudu
{
    static glm::vec2 m_mousePosition;
    static glm::vec2 m_mousePositionDelta;
    static glm::vec2 m_mouseScrollDelta;
    static float m_mouseWheelDelta;
    static GLFWwindow* m_window;

    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        m_mousePositionDelta = glm::vec2(xpos, ypos) - m_mousePosition;
        m_mousePosition = {xpos, ypos};
    }

    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        m_mouseScrollDelta = {xoffset, yoffset};
        m_mouseWheelDelta = yoffset;
    }

    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
    }


    void Input::Init(GLFWwindow* window)
    {
        m_window = window;

        glfwSetCursorPosCallback(m_window, CursorPosCallback);
        glfwSetScrollCallback(m_window, ScrollCallback);
        glfwSetKeyCallback(m_window, KeyCallback);
    }

    glm::vec2 Input::GetMousePosition()
    {
        return m_mousePosition;
    }

    glm::vec2 Input::GetMousePositionDelta()
    {
        return m_mousePositionDelta;
    }

    float Input::GetMouseWheelDelta()
    {
        return m_mouseWheelDelta;
    }

    bool Input::IsMouseButtonPressed(MouseButton button)
    {
        return glfwGetMouseButton(m_window, static_cast<int>(button)) == GLFW_PRESS;
    }

    bool Input::IsKeyPressed(KeyCode key)
    {
        return glfwGetKey(m_window, static_cast<int>(key)) == GLFW_PRESS;
    }

    glm::vec2 Input::GetMouseScrollDelta()
    {
        return m_mouseScrollDelta;
    }

    void Input::ResetDelta()
    {
        m_mousePositionDelta = glm::vec2(0.0f);
        m_mouseScrollDelta = glm::vec2(0.0f);
        m_mouseWheelDelta = 0.0f;
    }
}
