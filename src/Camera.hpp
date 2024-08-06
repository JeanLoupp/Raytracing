#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {

public:
    void mouse_button_callback(int button, int action);

    void mouse_callback(double xpos, double ypos);

    void scroll_callback(double xoffset, double yoffset);

    glm::mat4 getViewMat() const {
        return glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);
    }

    glm::mat4 getProjMat(float SCR_WIDTH, float SCR_HEIGHT) const {
        return glm::perspective(glm::radians(fov), SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
    }

    const glm::vec3 &getPos() const { return cameraPos; }

private:
    float lastX = 0;
    float lastY = 0;
    float yaw = 90.0f;
    float pitch = 0.0f;
    float fov = 45.0f;
    float cameraSpeed = 2.5f;
    float radius = 10.0f;
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, radius);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    bool firstMouse = true;
    bool mouseButtonPressed = false;
};

#endif // CAMERA_HPP