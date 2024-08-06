#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <memory>

#include "shaderProgram.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Transformation.hpp"
#include "ObjectsManager.hpp"
#include "utils.hpp"
#include "UserInterface.hpp"

// Constants and global variables
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

char scenePath[64] = "scene.scene";

const int UIwidth = 300;

Camera camera;
GLFWwindow *window;
bool wireframeMode = false;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

// Callback functions
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    width -= UIwidth;
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (ImGui::IsAnyItemHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
        if (action == GLFW_RELEASE) camera.mouse_button_callback(button, action);
        return;
    }
    camera.mouse_button_callback(button, action);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (ImGui::IsAnyItemHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
        return;
    }
    camera.mouse_callback(xpos, ypos);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (ImGui::IsAnyItemHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
        return;
    }
    camera.scroll_callback(xoffset, yoffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        wireframeMode = !wireframeMode;
        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}

// inits
void initGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

bool initWindow() {
    window = glfwCreateWindow(SCR_WIDTH + UIwidth, SCR_HEIGHT, "My Project", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    return true;
}

bool initOpenGL() {
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    return true;
}

bool init() {
    initGLFW();
    if (!initWindow()) return false;
    if (!initOpenGL()) return false;
    return true;
}

void beginRender(unsigned int shaderProgram) {
    // Clear window
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // Transformation
    glm::mat4 view = camera.getViewMat();
    glm::mat4 projection = camera.getProjMat(SCR_WIDTH, SCR_HEIGHT);

    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(camera.getPos()));

    // Light
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, &lightPos[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, &lightColor[0]);
}

int main() {
    if (!init()) return -1;

    unsigned int shaderProgram = createShaderProgram("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");

    ObjectManager objManager;
    objManager.loadMeshes();
    objManager.loadScene(scenePath);

    UserInterface UI(window, UIwidth, scenePath, &objManager);

    // objManager.addObject(Material(glm::vec3(1.0f, 0.5f, 0.2f), Transformation::PositionX(-1.0f)), sphereMeshIdx);
    // objManager.addObject(Material(glm::vec3(0.2f, 0.5f, 0.1f), Transformation::PositionX(1.0f)), sphereMeshIdx);
    // objManager.addObject(Material(), "Cube");
    // objManager.addObject(Material(glm::vec3(0.2f, 0.5f, 0.1f), Transformation::Scale(5.0f)), "Box");

    // Boucle de rendu
    while (!glfwWindowShouldClose(window)) {

        beginRender(shaderProgram);

        objManager.drawAll(shaderProgram);

        UI.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
