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

#include "ShaderProgram.hpp"
#include "ComputeShader.hpp"
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

bool useRaytracing = false;

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
    } else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        useRaytracing = !useRaytracing;
    }
}

GLuint genTexture(int width, int height) {
    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    return texture;
}

// inits
void initGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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

void beginRender(ShaderProgram &shaderProgram) {

    shaderProgram.use();

    shaderProgram.set("view", camera.getViewMat());
    shaderProgram.set("projection", camera.getProjMat(SCR_WIDTH, SCR_HEIGHT));
    shaderProgram.set("viewPos", camera.getPos());

    // Light
    shaderProgram.set("lightPos", lightPos);
    shaderProgram.set("lightColor", lightColor);
}

int main() {
    if (!init()) return -1;

    ShaderProgram shaderProgram("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    ShaderProgram RTshaderProgram("shaders/vert_shader_rt.glsl", "shaders/frag_shader_rt.glsl");

    std::shared_ptr<Mesh> quadMesh = Mesh::createQuad();

    ComputeShader computeShaderProgram("shaders/compute_shader.glsl");

    GLuint texOutput = genTexture(SCR_WIDTH, SCR_HEIGHT);

    ObjectManager objManager;
    objManager.loadMeshes();
    objManager.loadScene(scenePath);

    UserInterface UI(window, UIwidth, scenePath, &objManager);

    // Boucle de rendu
    while (!glfwWindowShouldClose(window)) {

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!useRaytracing) {
            beginRender(shaderProgram);

            objManager.drawAll(shaderProgram);

        } else {

            computeShaderProgram.use();

            glBindImageTexture(0, texOutput, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

            computeShaderProgram.set("width", SCR_WIDTH);
            computeShaderProgram.set("height", SCR_HEIGHT);

            // Launch compute shader
            glDispatchCompute((SCR_WIDTH + 15) / 16, (SCR_HEIGHT + 15) / 16, 1);

            // Wait for the compute shader to stop
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            RTshaderProgram.use();

            RTshaderProgram.set("renderedImage", 0);

            glBindVertexArray(quadMesh->getVAO());
            glDrawElements(GL_TRIANGLES, quadMesh->getIndexCount(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        UI.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteTextures(1, &texOutput);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
