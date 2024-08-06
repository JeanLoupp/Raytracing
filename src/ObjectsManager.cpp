#include "ObjectsManager.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

void ObjectManager::addMesh(std::shared_ptr<Mesh> mesh) {
    meshes.push_back(mesh);
    objectsPerMesh.push_back(std::vector<int>());
    meshNames.push_back(mesh->getName());
    meshNamesMap[mesh->getName()] = meshes.size() - 1;
}

void ObjectManager::loadMeshes() {
    addMesh(Mesh::createCube());
    addMesh(Mesh::createSphere());
    addMesh(Mesh::createBox());
    addMesh(Mesh::createPlane());
}

int ObjectManager::addObject(Material material, unsigned int meshIdx) {
    objects.push_back(material);
    objectsPerMesh[meshIdx].push_back(objects.size() - 1);
    idxToMesh.push_back(std::make_pair(meshIdx, objectsPerMesh[meshIdx].size() - 1));
    genNames();
    return objects.size() - 1;
}

int ObjectManager::addObject(Material material, std::string meshName) {
    const auto &it = meshNamesMap.find(meshName);
    if (it == meshNamesMap.end()) std::cerr << "Invalid meshName in addObject: " << meshName << std::endl;
    return addObject(material, it->second);
}

int ObjectManager::addObject(unsigned int meshIdx) {
    objects.emplace_back();
    objectsPerMesh[meshIdx].push_back(objects.size() - 1);
    idxToMesh.push_back(std::make_pair(meshIdx, objectsPerMesh[meshIdx].size() - 1));
    genNames();
    return objects.size() - 1;
}

int ObjectManager::addObject(std::string meshName) {
    const auto &it = meshNamesMap.find(meshName);
    if (it == meshNamesMap.end()) std::cerr << "Invalid meshName in addObject: " << meshName << std::endl;
    return addObject(it->second);
}

void ObjectManager::drawAll(unsigned int shaderProgram) {
    glUseProgram(shaderProgram);
    for (int i = 0; i < meshes.size(); i++) {
        glBindVertexArray(meshes[i]->getVAO());

        for (int matIdx : objectsPerMesh[i]) {

            int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
            glUniform3fv(colorLoc, 1, glm::value_ptr(objects[matIdx].getColor()));

            unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(objects[matIdx].getModel()));

            glDrawElements(GL_TRIANGLES, meshes[i]->getIndexCount(), GL_UNSIGNED_INT, 0);
        }

        glBindVertexArray(0);
    }
}

void ObjectManager::genNames() {
    names.clear();
    for (int i = 0; i < objects.size(); i++) {
        names.push_back(meshes[idxToMesh[i].first]->getName() + " " + std::to_string(idxToMesh[i].second));
    }
}

void ObjectManager::genMeshNames() {
    meshNames.clear();
    for (int i = 0; i < meshes.size(); i++) {
        meshNames.push_back(meshes[i]->getName());
    }
}

void ObjectManager::removeObject(int idx) {

    int meshIdx = idxToMesh[idx].first;
    int objIdxInMesh = idxToMesh[idx].second;

    objects.erase(objects.begin() + idx);

    for (int i = idx + 1; i < idxToMesh.size(); i++) {
        objectsPerMesh[idxToMesh[i].first][idxToMesh[i].second]--;
    }

    objectsPerMesh[meshIdx].erase(objectsPerMesh[meshIdx].begin() + objIdxInMesh);

    for (int i = 0; i < idxToMesh.size(); i++) {

        if (idxToMesh[i].first == meshIdx && idxToMesh[i].second > objIdxInMesh)
            idxToMesh[i].second--;
    }

    idxToMesh.erase(idxToMesh.begin() + idx);

    genNames();
}

void ObjectManager::saveScene(const std::string &filename) {
    std::ofstream outfile("data/scenes/" + filename);
    outfile << objects.size() << "\n";
    if (!outfile.is_open()) {
        std::cerr << "Erreur lors de l'ouverture du fichier pour l'écriture.\n";
        return;
    }
    for (int i = 0; i < objects.size(); i++) {
        outfile << "MESH " << meshNames[idxToMesh[i].first] << "\n";
        outfile << "POS " << objects[i].getPos().x << " " << objects[i].getPos().y << " " << objects[i].getPos().z << "\n";
        outfile << "COLOR " << objects[i].getColor().r << " " << objects[i].getColor().g << " " << objects[i].getColor().b << "\n";
        outfile << "SIZE " << objects[i].getSize().x << " " << objects[i].getSize().y << " " << objects[i].getSize().z << "\n";
        outfile << "ROTATION " << objects[i].getRotation().x << " " << objects[i].getRotation().y << " " << objects[i].getRotation().z << "\n";
        outfile << ".\n";
    }
    outfile.close();
}

void ObjectManager::loadScene(const std::string &filename) {
    std::ifstream infile("data/scenes/" + filename);
    if (!infile) {
        std::cerr << "Erreur lors de l'ouverture du fichier !" << std::endl;
        return;
    }

    std::string word;

    std::string mesh;
    glm::vec3 color, pos, size, rotation;

    while (infile >> word) {
        if (word == "MESH") {
            infile >> mesh;
        } else if (word == "POS") {
            infile >> pos.x >> pos.y >> pos.z;
        } else if (word == "COLOR") {
            infile >> color.x >> color.y >> color.z;
        } else if (word == "SIZE") {
            infile >> size.x >> size.y >> size.z;
        } else if (word == "ROTATION") {
            infile >> rotation.x >> rotation.y >> rotation.z;
        } else if (word == ".") {
            addObject(Material(color, Transformation(pos, size, rotation)), mesh);
        }
    }

    infile.close();
}
