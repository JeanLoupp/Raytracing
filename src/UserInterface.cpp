#include "UserInterface.hpp"

#include <GLFW/glfw3.h>

UserInterface::UserInterface(GLFWwindow *window, int UIwidth, char filename[], ObjectManager *objManager)
    : window(window), UIwidth(UIwidth), objManager(objManager) {

    strncpy(UI_filename, filename, 64);
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void UserInterface::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    int window_width, window_height;
    glfwGetWindowSize(window, &window_width, &window_height);

    ImGui::SetNextWindowPos(ImVec2(window_width - UIwidth, 0));
    ImGui::SetNextWindowSize(ImVec2(UIwidth, window_height));

    ImGui::Begin("0", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

    if (ImGui::Button("Edit scene")) {
        page = 0;
    }
    ImGui::SameLine();
    if (ImGui::Button("Edit render")) {
        page = 1;
    }

    if (page == 0) {

        auto &meshNames = objManager->getMeshNames();

        if (ImGui::BeginCombo("##addObj", "Add an Object")) {
            for (int n = 0; n < meshNames.size(); n++) {
                if (ImGui::Selectable(meshNames[n].c_str(), false)) {
                    UI_selectedObj = objManager->addObject(n);
                    UI_isModified = true;
                    UI_shouldReset = true;
                    UI_resetTriangleBuff = true;
                }
            }
            ImGui::EndCombo();
        }

        auto &names = objManager->getNames();

        if (names.size() > 0) {

            if (ImGui::BeginCombo("##selectObj", names[UI_selectedObj].c_str())) {
                for (int n = 0; n < names.size(); n++) {
                    bool is_selected = (UI_selectedObj == n);
                    if (ImGui::Selectable(names[n].c_str(), is_selected))
                        UI_selectedObj = n;

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            Material &selectedObj = objManager->getObject(UI_selectedObj);

            glm::vec3 position = selectedObj.getPos();
            glm::vec3 color = selectedObj.getColor();
            glm::vec3 emiColor = selectedObj.getEmiColor();
            glm::vec3 rotation = selectedObj.getRotation();
            glm::vec3 size = selectedObj.getSize();
            float smoothness = selectedObj.getSmoothness();
            float reflexivity = selectedObj.getReflexivity();

            float posArray[3] = {position.x, position.y, position.z};
            float colorArray[3] = {color.r, color.g, color.b};
            float emiColorArray[3] = {emiColor.r, emiColor.g, emiColor.b};
            float rotArray[3] = {rotation.x, rotation.y, rotation.z};
            float sizeArray[3] = {size.x, size.y, size.z};
            float uniformScale = size.x;

            if (ImGui::ColorEdit3("Color", colorArray)) {
                selectedObj.setColor(glm::vec3(colorArray[0], colorArray[1], colorArray[2]));
                UI_isModified = true;
                UI_shouldReset = true;
            }

            if (ImGui::ColorEdit3("Emiss. Color", emiColorArray)) {
                selectedObj.setEmiColor(glm::vec3(emiColorArray[0], emiColorArray[1], emiColorArray[2]));
                UI_isModified = true;
                UI_shouldReset = true;
            }

            if (ImGui::SliderFloat("Smoothness", &smoothness, 0.0f, 1.0f, "%.2f")) {
                selectedObj.setSmoothness(smoothness);
                UI_isModified = true;
                UI_shouldReset = true;
            }

            if (ImGui::SliderFloat("Reflexivity", &reflexivity, 0.0f, 1.0f, "%.2f")) {
                selectedObj.setReflexivity(reflexivity);
                UI_isModified = true;
                UI_shouldReset = true;
            }

            ImGui::Text("Position");
            bool updatePos = false;
            if (ImGui::DragFloat("X##pos", &posArray[0], 0.01f, 0.0f, 0.0f, "%.2f")) updatePos = true;
            if (ImGui::DragFloat("Y##pos", &posArray[1], 0.01f, 0.0f, 0.0f, "%.2f")) updatePos = true;
            if (ImGui::DragFloat("Z##pos", &posArray[2], 0.01f, 0.0f, 0.0f, "%.2f")) updatePos = true;
            if (updatePos) {
                selectedObj.setPos(glm::vec3(posArray[0], posArray[1], posArray[2]));
                UI_isModified = true;
                UI_shouldReset = true;
            }

            ImGui::Text("Rotation");
            bool updateRot = false;
            if (ImGui::DragFloat("X##rot", &rotArray[0], 0.2f, -360.0f, 360.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp)) updateRot = true;
            if (ImGui::DragFloat("Y##rot", &rotArray[1], 0.2f, -360.0f, 360.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp)) updateRot = true;
            if (ImGui::DragFloat("Z##rot", &rotArray[2], 0.2f, -360.0f, 360.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp)) updateRot = true;
            if (updateRot) {
                selectedObj.setRotation(glm::vec3(rotArray[0], rotArray[1], rotArray[2]));
                UI_isModified = true;
                UI_shouldReset = true;
            }

            ImGui::Text("Scale");
            ImGui::Checkbox("Uniform Size", &UI_uniformSize);
            if (UI_uniformSize) {
                if (ImGui::DragFloat("Scale", &uniformScale, 0.01f, 0.0f, 0.0f, "%.2f")) {
                    selectedObj.setSize(uniformScale);
                    UI_isModified = true;
                    UI_shouldReset = true;
                }
            } else {
                bool updateScale = false;
                if (ImGui::DragFloat("X##scale", &sizeArray[0], 0.01f, 0.0f, 0.0f, "%.2f")) updateScale = true;
                if (ImGui::DragFloat("Y##scale", &sizeArray[1], 0.01f, 0.0f, 0.0f, "%.2f")) updateScale = true;
                if (ImGui::DragFloat("Z##scale", &sizeArray[2], 0.01f, 0.0f, 0.0f, "%.2f")) updateScale = true;
                if (updateScale) {
                    selectedObj.setSize(glm::vec3(sizeArray[0], sizeArray[1], sizeArray[2]));
                    UI_isModified = true;
                    UI_shouldReset = true;
                }
            }

            if (ImGui::Button("Delete")) {
                objManager->removeObject(UI_selectedObj);
                UI_selectedObj = 0;
                UI_isModified = true;
                UI_shouldReset = true;
                UI_resetTriangleBuff = true;
            }

            float button_height_with_spacing = ImGui::GetFrameHeightWithSpacing() * 3;
            float remaining_height = ImGui::GetContentRegionAvail().y - button_height_with_spacing;

            if (remaining_height > 0.0f) {
                ImGui::Dummy(ImVec2(0.0f, remaining_height));
            }

            ImGui::Text("Save");
            ImGui::InputText("File name", UI_filename, 64);
            if (ImGui::Button("Save")) {
                objManager->saveScene(UI_filename);
                UI_isModified = false;
            }

            ImGui::SameLine();

            if (UI_isModified) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not saved");
            }
        }
    } else if (page == 1) {
        int maxBounces = objManager->getMaxBounces();
        if (ImGui::DragInt("maxBounces", &maxBounces, 0.1f, 1.0f, 50.0f, "%d", ImGuiSliderFlags_AlwaysClamp)) {
            objManager->setMaxBounces(maxBounces);
            UI_shouldReset = true;
        }
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool UserInterface::shouldReset() {
    if (UI_shouldReset) {
        UI_shouldReset = false;
        return true;
    }
    return false;
}

bool UserInterface::shouldResetTriBuff() {
    if (UI_resetTriangleBuff) {
        UI_resetTriangleBuff = false;
        return true;
    }
    return false;
}