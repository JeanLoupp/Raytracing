#ifndef OBJECT_MANAGER_HPP
#define OBJECT_MANAGER_HPP

#include <utility>
#include <unordered_map>

#include "Material.hpp"
#include "ShaderProgram.hpp"

class ObjectManager {
public:
    void addMesh(std::shared_ptr<Mesh>);
    int addObject(Material material, unsigned int meshIdx);
    int addObject(Material material, const std::string &meshName);
    int addObject(unsigned int meshIdx);
    int addObject(const std::string &meshName);
    void drawAll(ShaderProgram &shaderProgram);
    void genNames();
    void genMeshNames();
    std::vector<std::string> &getNames() { return names; };
    std::vector<std::string> &getMeshNames() { return meshNames; };
    Material &getObject(int idx) { return objects[idx]; }
    void removeObject(int idx);
    void loadMeshes();

    const std::vector<int> &getObjectsPerMesh(const std::string &meshName);
    const std::vector<int> &getObjectsPerMesh(unsigned int meshIdx);

    int getMaxBounces() const { return maxBounces; }
    void setMaxBounces(const int bounces) { maxBounces = bounces; }

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

    int maxBounces = 5;
};

#endif // OBJECT_MANAGER_HPP