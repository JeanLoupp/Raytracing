#include "Camera.hpp"

#include <iostream>

void Camera::mouse_button_callback(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouseButtonPressed = true;
        firstMouse = true;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mouseButtonPressed = false;
    }
}

void Camera::mouse_callback(double xpos, double ypos) {

    if (mouseButtonPressed) {

        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (glm::abs(pitch) > 80.0f) pitch = 80.0f * glm::sign(pitch);

        cameraPos.x = radius * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraPos.y = radius * sin(glm::radians(pitch));
        cameraPos.z = radius * sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    }
}

void Camera::scroll_callback(double xoffset, double yoffset) {
    radius -= (float)yoffset * 0.2;
    if (radius < 0.0f) radius = 0.0f;
    cameraPos.x = radius * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraPos.y = radius * sin(glm::radians(pitch));
    cameraPos.z = radius * sin(glm::radians(yaw)) * cos(glm::radians(pitch));
}