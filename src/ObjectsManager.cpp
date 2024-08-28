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
    addMesh(Mesh::createTore());
}

int ObjectManager::addObject(Material material, unsigned int meshIdx) {
    objects.push_back(material);
    objectsPerMesh[meshIdx].push_back(objects.size() - 1);
    idxToMesh.push_back(std::make_pair(meshIdx, objectsPerMesh[meshIdx].size() - 1));
    genNames();
    return objects.size() - 1;
}

int ObjectManager::addObject(Material material, const std::string &meshName) {
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

int ObjectManager::addObject(const std::string &meshName) {
    const auto &it = meshNamesMap.find(meshName);
    if (it == meshNamesMap.end()) std::cerr << "Invalid meshName in addObject: " << meshName << std::endl;
    return addObject(it->second);
}

const std::vector<int> &ObjectManager::getObjectsPerMesh(unsigned int meshIdx) {
    return objectsPerMesh[meshIdx];
}

const std::vector<int> &ObjectManager::getObjectsPerMesh(const std::string &meshName) {
    const auto &it = meshNamesMap.find(meshName);
    if (it == meshNamesMap.end()) std::cerr << "Invalid meshName in getObjectsPerMesh: " << meshName << std::endl;
    return getObjectsPerMesh(it->second);
}

void ObjectManager::drawAll(ShaderProgram &shaderProgram) {
    shaderProgram.use();
    for (int i = 0; i < meshes.size(); i++) {
        glBindVertexArray(meshes[i]->getVAO());

        for (int matIdx : objectsPerMesh[i]) {

            shaderProgram.set("objectColor", objects[matIdx].getColor());
            shaderProgram.set("model", objects[matIdx].getModel());

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
        outfile << "COLOR " << objects[i].getColor().r << " " << objects[i].getColor().g << " " << objects[i].getColor().b << "\n";
        outfile << "EMICOLOR " << objects[i].getEmiColor().r << " " << objects[i].getEmiColor().g << " " << objects[i].getEmiColor().b << "\n";
        outfile << "SMOOTHNESS " << objects[i].getSmoothness() << "\n";
        outfile << "REFLEXIVITY " << objects[i].getReflexivity() << "\n";
        outfile << "POS " << objects[i].getPos().x << " " << objects[i].getPos().y << " " << objects[i].getPos().z << "\n";
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
    glm::vec3 color(0.0f), emiColor(0.0f), pos(0.0f), size(1.0f), rotation(0.0f);
    float smoothness = 1.0f;
    float reflexivity = 0.0f;

    while (infile >> word) {
        if (word == "MESH") {
            infile >> mesh;
        } else if (word == "COLOR") {
            infile >> color.x >> color.y >> color.z;
        } else if (word == "EMICOLOR") {
            infile >> emiColor.x >> emiColor.y >> emiColor.z;
        } else if (word == "SMOOTHNESS") {
            infile >> smoothness;
        } else if (word == "REFLEXIVITY") {
            infile >> reflexivity;
        } else if (word == "POS") {
            infile >> pos.x >> pos.y >> pos.z;
        } else if (word == "SIZE") {
            infile >> size.x >> size.y >> size.z;
        } else if (word == "ROTATION") {
            infile >> rotation.x >> rotation.y >> rotation.z;
        } else if (word == ".") {
            addObject(Material(color, emiColor, smoothness, reflexivity, Transformation(pos, size, rotation)), mesh);
        }
    }

    infile.close();
}

void ObjectManager::genAllTriangles() {
    trianglesBuffer.clear();
    triangleToMat.clear();

    std::vector<glm::vec3> transVertices;
    std::vector<glm::vec3> transNormals;

    std::vector<std::string> triangleMeshes = {"Plane", "Cube", "Box"};
    for (std::string &meshName : triangleMeshes) {

        int meshIdx = meshNamesMap[meshName];

        const std::vector<glm::vec3> &vertices = meshes[meshIdx]->getVertices();
        const std::vector<glm::vec3> &normals = meshes[meshIdx]->getNormals();
        const std::vector<unsigned int> &indices = meshes[meshIdx]->getIndices();

        for (int idx : getObjectsPerMesh(meshName)) {

            triangleToMat.emplace_back(idx, trianglesBuffer.size(), trianglesBuffer.size() + indices.size() / 3);

            transVertices.resize(vertices.size());
            transNormals.resize(normals.size());

            const glm::mat4 &model = objects[idx].getModel();
            glm::mat3 modelNorm = glm::mat3(transpose(inverse(model)));

            for (int i = 0; i < vertices.size(); i++) {
                transVertices[i] = glm::vec3(model * glm::vec4(vertices[i], 1.0));
                // TODO: one normal per vertex
                transNormals[i] = normalize(glm::vec3(modelNorm * normals[i]));
            }
            for (int i = 0; i < indices.size(); i += 3) {
                // TODO: one normal per vertex
                trianglesBuffer.emplace_back(transVertices[indices[i]], transVertices[indices[i + 1]], transVertices[indices[i + 2]], transNormals[indices[i]]);
            }
        }
    }
}
