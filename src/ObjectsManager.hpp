#ifndef OBJECT_MANAGER_HPP
#define OBJECT_MANAGER_HPP

#include <utility>
#include <unordered_map>

#include "Material.hpp"

class ObjectManager {
public:
    void addMesh(std::shared_ptr<Mesh>);
    int addObject(Material material, unsigned int meshIdx);
    int addObject(Material material, std::string meshName);
    int addObject(unsigned int meshIdx);
    int addObject(std::string meshName);
    void drawAll(unsigned int shaderProgram);
    void genNames();
    void genMeshNames();
    std::vector<std::string> &getNames() { return names; };
    std::vector<std::string> &getMeshNames() { return meshNames; };
    Material &getObject(int idx) { return objects[idx]; }
    void removeObject(int idx);
    void loadMeshes();

    void saveScene(const std::string &filename);
    void loadScene(const std::string &filename);

private:
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<Material> objects;
    std::vector<std::vector<int>> objectsPerMesh;
    std::vector<std::pair<int, int>> idxToMesh;
    std::vector<std::string> names;
    std::vector<std::string> meshNames;
    std::unordered_map<std::string, int> meshNamesMap;
};

#endif // OBJECT_MANAGER_HPP