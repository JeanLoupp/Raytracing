#ifndef MESH_HPP
#define MESH_HPP

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>

class Mesh {
public:
    Mesh(const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &normals, const std::vector<unsigned int> &indices, std::string name = "");
    Mesh(const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &normals, const std::vector<glm::vec2> &textures, const std::vector<unsigned int> &indices, std::string name = "");

    void draw(unsigned int shaderProgram, glm::vec3 &color, glm::mat4 &modelMat);

    ~Mesh();

    static std::shared_ptr<Mesh> createCube();
    static std::shared_ptr<Mesh> createSphere(int resolution = 16);
    static std::shared_ptr<Mesh> createPlane();
    static std::shared_ptr<Mesh> createBox();
    static std::shared_ptr<Mesh> createQuad();
    static std::shared_ptr<Mesh> createTore(int resolution = 16);

    const unsigned int getVAO() const { return VAO; }
    const unsigned int getIndexCount() const { return indexCount; }

    void setName(std::string newName) { name = newName; }
    std::string getName() const { return name; }

    const std::vector<glm::vec3> &getVertices() const { return vertices; }
    const std::vector<glm::vec3> &getNormals() const { return normals; }
    const std::vector<unsigned int> &getIndices() const { return indices; }

private:
    unsigned int VAO, VBO, NBO, TBO, EBO;
    bool hasNormals, hasTextures;
    size_t indexCount;
    std::string name;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
};

#endif // MESH_HPP
