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

    void draw(unsigned int shaderProgram, glm::vec3 &color, glm::mat4 &modelMat);

    ~Mesh();

    static std::shared_ptr<Mesh> createCube();
    static std::shared_ptr<Mesh> createSphere(int resolution = 16);
    static std::shared_ptr<Mesh> createPlane();
    static std::shared_ptr<Mesh> createBox();

    const unsigned int getVAO() const { return VAO; }
    const unsigned int getIndexCount() const { return indexCount; }

    void setName(std::string newName) { name = newName; }
    std::string getName() const { return name; }

private:
    unsigned int VAO, VBO, NBO, EBO;
    size_t indexCount;
    std::string name;
};

#endif // MESH_HPP
