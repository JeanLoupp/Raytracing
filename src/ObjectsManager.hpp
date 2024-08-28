#ifndef OBJECT_MANAGER_HPP
#define OBJECT_MANAGER_HPP

#include <utility>
#include <unordered_map>

#include "Material.hpp"
#include "ShaderProgram.hpp"

struct Triangle {
    glm::vec3 v0;
    float pad0; // Explicit 4 bytes aligment
    glm::vec3 v1;
    float pad1; // Explicit 4 bytes aligment
    glm::vec3 v2;
    float pad2; // Explicit 4 bytes aligment
    glm::vec3 normal;
    float pad3; // Explicit 4 bytes aligment

    Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 normal)
        : v0(v0), pad0(0.0f), v1(v1), pad1(0.0f), v2(v2), pad2(0.0f), normal(normal), pad3(0.0f) {}
};

struct TriangleMeshInfo {
    int matIdx;
    int startIdx;
    int endIdx;

    TriangleMeshInfo(int matIdx, int startIdx, int endIdx) : matIdx(matIdx), startIdx(startIdx), endIdx(endIdx) {}
};

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

    void genAllTriangles();
    const std::vector<Triangle> &getTriangles() const { return trianglesBuffer; };
    const std::vector<TriangleMeshInfo> &getTriangleToObject() const { return triangleToMat; };

private:
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<Material> objects;
    std::vector<std::vector<int>> objectsPerMesh;
    std::vector<std::pair<int, int>> idxToMesh;
    std::vector<std::string> names;
    std::vector<std::string> meshNames;
    std::unordered_map<std::string, int> meshNamesMap;

    int maxBounces = 5;

    std::vector<Triangle> trianglesBuffer;
    std::vector<TriangleMeshInfo> triangleToMat;
};

#endif // OBJECT_MANAGER_HPP