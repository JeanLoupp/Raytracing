#ifndef USERINTERFACE_HPP
#define USERINTERFACE_HPP

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ObjectsManager.hpp"

class UserInterface {
public:
    UserInterface(GLFWwindow *window, int UIwidth, char filename[], ObjectManager *objManager);
    void render();

private:
    int UI_selectedObj = 0;
    bool UI_uniformSize = false;
    bool UI_showOpen = false;
    bool UI_isModified = false;
    char UI_filename[64];

    int UIwidth;
    GLFWwindow *window;
    ObjectManager *objManager;
};

#endif // USERINTERFACE_HPP
