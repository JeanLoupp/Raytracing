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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Constants and global variables
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 800;

const int textureWidth = 1600;
const int textureHeight = 1600;

GLuint texOutput1;
GLuint texOutput2;

char scenePath[64] = "scene.scene";

const int UIwidth = 300;

Camera camera;
GLFWwindow *window;
bool wireframeMode = false;

glm::vec3 lightPos(0.0f, 2.0f, 2.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

bool useRaytracing = false;

// Fonction pour sauvegarder l'écran
void SaveScreenshot(const char *filename, int width, int height) {
    std::vector<unsigned char> pixels(width * height * 3); // RGB

    // Lire les pixels de la fenêtre actuelle
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Inverser l'image verticalement
    for (int y = 0; y < height / 2; ++y) {
        for (int x = 0; x < width * 3; ++x) {
            std::swap(pixels[y * width * 3 + x], pixels[(height - y - 1) * width * 3 + x]);
        }
    }

    // Sauvegarder l'image au format PNG
    if (stbi_write_png(filename, width, height, 3, pixels.data(), width * 3)) {
        std::cout << "Screenshot saved to " << filename << std::endl;
    } else {
        std::cout << "Failed to save screenshot" << std::endl;
    }
}

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
    } else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        SaveScreenshot("data/output/texture.png", SCR_WIDTH + UIwidth, SCR_HEIGHT);
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

    return texture;
}

GLuint genTrianglesSSBO(const std::vector<Triangle> &triangles) {

    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, triangles.size() * sizeof(Triangle), triangles.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo); // binding 1 in compute shader
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    return ssbo;
}

GLuint resetTrianglesSSBO(GLuint &oldSsbo, const std::vector<Triangle> &triangles) {
    glDeleteBuffers(1, &oldSsbo);
    return genTrianglesSSBO(triangles);
}

void updateTrianglesSSBO(GLuint &ssbo, const std::vector<Triangle> &triangles) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, triangles.size() * sizeof(Triangle), triangles.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// inits
void initGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

bool initWindow() {
    window = glfwCreateWindow(SCR_WIDTH + UIwidth, SCR_HEIGHT, "Raytracing", nullptr, nullptr);
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

    ///////////////////

    // Taille maximale pour chaque dimension X, Y, Z
    GLint workGroupSize[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workGroupSize[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workGroupSize[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workGroupSize[2]);

    // Nombre total maximal d'invocations
    GLint maxWorkGroupInvocations;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxWorkGroupInvocations);

    // Affichage
    std::cout << "Max work group size: X=" << workGroupSize[0]
              << ", Y=" << workGroupSize[1]
              << ", Z=" << workGroupSize[2] << std::endl;

    std::cout << "Max work group invocations: " << maxWorkGroupInvocations << std::endl;

    ///////////////////////

    ShaderProgram shaderProgram("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    ShaderProgram RTshaderProgram("shaders/vert_shader_rt.glsl", "shaders/frag_shader_rt.glsl");

    std::shared_ptr<Mesh> quadMesh = Mesh::createQuad();

    ComputeShader computeShaderProgram("shaders/compute_shader.glsl");

    GLuint texOutput1 = genTexture(textureWidth, textureHeight);
    GLuint texOutput2 = genTexture(textureWidth, textureHeight);

    ObjectManager objManager;
    objManager.loadMeshes();
    objManager.loadScene(scenePath);

    objManager.genAllTriangles();
    GLuint ssboTri = genTrianglesSSBO(objManager.getTriangles());

    UserInterface UI(window, UIwidth, scenePath, &objManager);

    int frameCount = 0;

    // Boucle de rendu
    while (!glfwWindowShouldClose(window)) {

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (UI.shouldResetTriBuff()) {
            objManager.genAllTriangles();
            ssboTri = resetTrianglesSSBO(ssboTri, objManager.getTriangles());
            frameCount = 0;
            UI.shouldReset();
        } else if (UI.shouldReset()) {
            frameCount = 0;
            objManager.genAllTriangles(); // TODO: only update when model matrix is changed
            updateTrianglesSSBO(ssboTri, objManager.getTriangles());
        }

        if (camera.hasMoved()) frameCount = 0;

        if (!useRaytracing) {
            beginRender(shaderProgram);

            objManager.drawAll(shaderProgram);

        } else {

            computeShaderProgram.use();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texOutput1);
            computeShaderProgram.set("prevImage", 0);

            glBindImageTexture(0, texOutput2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

            computeShaderProgram.set("frameCount", frameCount);
            computeShaderProgram.set("maxBounces", objManager.getMaxBounces());

            computeShaderProgram.set("width", (int)textureWidth);
            computeShaderProgram.set("height", (int)textureHeight);
            computeShaderProgram.set("cameraPosition", camera.getPos());
            computeShaderProgram.set("viewMatrix", camera.getViewMat());

            std::vector<int> spheres = objManager.getObjectsPerMesh("Sphere");
            for (int i = 0; i < spheres.size(); i++) {
                computeShaderProgram.setArray("spheres", i, "pos", objManager.getObject(spheres[i]).getPos());
                computeShaderProgram.setArray("spheres", i, "r", objManager.getObject(spheres[i]).getSize()[0]);
                computeShaderProgram.setArray("spheres", i, "mat.color", objManager.getObject(spheres[i]).getColor());
                computeShaderProgram.setArray("spheres", i, "mat.emissionColor", objManager.getObject(spheres[i]).getEmiColor());
                computeShaderProgram.setArray("spheres", i, "mat.emissionStrength", objManager.getObject(spheres[i]).getEmissionStrength());
                computeShaderProgram.setArray("spheres", i, "mat.smoothness", objManager.getObject(spheres[i]).getSmoothness());
                computeShaderProgram.setArray("spheres", i, "mat.reflexivity", objManager.getObject(spheres[i]).getReflexivity());
            }

            computeShaderProgram.set("sphereCount", (int)spheres.size());

            std::vector<int> tores = objManager.getObjectsPerMesh("Tore");
            for (int i = 0; i < tores.size(); i++) {
                computeShaderProgram.setArray("tores", i, "pos", objManager.getObject(tores[i]).getPos());
                computeShaderProgram.setArray("tores", i, "R", objManager.getObject(tores[i]).getSize()[0]);
                computeShaderProgram.setArray("tores", i, "r", 0.1f);
                computeShaderProgram.setArray("tores", i, "mat.color", objManager.getObject(tores[i]).getColor());
                computeShaderProgram.setArray("tores", i, "mat.emissionColor", objManager.getObject(tores[i]).getEmiColor());
                computeShaderProgram.setArray("tores", i, "mat.emissionStrength", objManager.getObject(tores[i]).getEmissionStrength());
                computeShaderProgram.setArray("tores", i, "mat.smoothness", objManager.getObject(tores[i]).getSmoothness());
                computeShaderProgram.setArray("tores", i, "mat.reflexivity", objManager.getObject(tores[i]).getReflexivity());
            }

            computeShaderProgram.set("toreCount", (int)tores.size());

            std::vector<TriangleMeshInfo> trianglesInfo = objManager.getTriangleToObject();
            for (int i = 0; i < trianglesInfo.size(); i++) {

                computeShaderProgram.setArray("triangleMeshes", i, "startIdx", trianglesInfo[i].startIdx);
                computeShaderProgram.setArray("triangleMeshes", i, "endIdx", trianglesInfo[i].endIdx);
                computeShaderProgram.setArray("triangleMeshes", i, "mat.color", objManager.getObject(trianglesInfo[i].matIdx).getColor());
                computeShaderProgram.setArray("triangleMeshes", i, "mat.emissionColor", objManager.getObject(trianglesInfo[i].matIdx).getEmiColor());
                computeShaderProgram.setArray("triangleMeshes", i, "mat.emissionStrength", objManager.getObject(trianglesInfo[i].matIdx).getEmissionStrength());
                computeShaderProgram.setArray("triangleMeshes", i, "mat.smoothness", objManager.getObject(trianglesInfo[i].matIdx).getSmoothness());
                computeShaderProgram.setArray("triangleMeshes", i, "mat.reflexivity", objManager.getObject(trianglesInfo[i].matIdx).getReflexivity());
            }

            computeShaderProgram.set("triangleMeshCount", (int)trianglesInfo.size());

            // Launch compute shader
            glDispatchCompute(textureWidth / 16, textureHeight / 16, 1);

            // Wait for the compute shader to stop
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            RTshaderProgram.use();

            glBindVertexArray(quadMesh->getVAO());

            RTshaderProgram.set("renderedImage", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texOutput1);

            glDrawElements(GL_TRIANGLES, quadMesh->getIndexCount(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            std::swap(texOutput1, texOutput2);
            frameCount++;
        }

        UI.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteTextures(1, &texOutput1);
    glDeleteTextures(1, &texOutput2);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
