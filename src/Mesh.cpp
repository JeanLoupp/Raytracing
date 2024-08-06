#include "Mesh.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <memory>

#define PI 3.14159265359

// Constructeur de la classe Mesh
Mesh::Mesh(const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &normals, const std::vector<unsigned int> &indices, std::string name)
    : indexCount(indices.size()), name(name) {

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &NBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

// Fonction pour dessiner le maillage
void Mesh::draw(unsigned int shaderProgram, glm::vec3 &color, glm::mat4 &modelMat) {
    glUseProgram(shaderProgram);

    int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));

    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// Destructeur de la classe Mesh
Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &NBO);
    glDeleteBuffers(1, &EBO);
}

// Fonction statique pour créer un cube
std::shared_ptr<Mesh> Mesh::createCube() {
    std::vector<glm::vec3> vertices = {
        // Arrière
        {-1.0f, -1.0f, -1.0f},
        {1.0f, -1.0f, -1.0f},
        {1.0f, 1.0f, -1.0f},
        {-1.0f, 1.0f, -1.0f},
        // Avant
        {-1.0f, -1.0f, 1.0f},
        {1.0f, -1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f},
        // Gauche
        {-1.0f, -1.0f, -1.0f},
        {-1.0f, 1.0f, -1.0f},
        {-1.0f, 1.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f},
        // Droite
        {1.0f, -1.0f, -1.0f},
        {1.0f, 1.0f, -1.0f},
        {1.0f, 1.0f, 1.0f},
        {1.0f, -1.0f, 1.0f},
        // Bas
        {-1.0f, -1.0f, -1.0f},
        {1.0f, -1.0f, -1.0f},
        {1.0f, -1.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f},
        // Haut
        {-1.0f, 1.0f, -1.0f},
        {1.0f, 1.0f, -1.0f},
        {1.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f}};

    std::vector<glm::vec3> normals = {
        // Normales
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f}, // Arrière
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f}, // Avant
        {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f}, // Gauche
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f}, // Droite
        {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f}, // Bas
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f} // Haut
    };

    std::vector<unsigned int> indices = {
        // Face arrière
        0, 2, 1, 3, 2, 0,
        // Face avant
        4, 5, 6, 6, 7, 4,
        // Face gauche
        9, 8, 10, 11, 10, 8,
        // Face droite
        12, 13, 14, 14, 15, 12,
        // Face inférieure
        16, 17, 18, 18, 19, 16,
        // Face supérieure
        21, 20, 22, 23, 22, 20};

    return std::make_shared<Mesh>(vertices, normals, indices, "Cube");
}

std::shared_ptr<Mesh> Mesh::createSphere(int resolution) {

    int total = resolution * resolution;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    glm::vec3 pos;
    float phi, theta;

    int triShift[6][2] = {{0, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0}, {1, 1}};

    for (int i = 0; i <= resolution; i++) {
        for (int j = 0; j <= resolution; j++) {
            phi = -i * 2 * PI / resolution; // col
            theta = j * PI / resolution;    // li

            pos[0] = glm::sin(theta) * glm::cos(phi);
            pos[2] = glm::sin(theta) * glm::sin(phi);
            pos[1] = glm::cos(theta);

            vertices.push_back(pos);
            normals.push_back(pos);

            if (i == resolution || j == resolution) continue;

            // Calculate triangle indices
            int nextRow = (i + 1);
            int nextCol = (j + 1);

            int current = j * (resolution + 1) + i;
            int next = j * (resolution + 1) + nextRow;
            int diagonal = nextCol * (resolution + 1) + i;
            int nextDiagonal = nextCol * (resolution + 1) + nextRow;

            // Triangle 1
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(diagonal);

            // Triangle 2
            indices.push_back(next);
            indices.push_back(nextDiagonal);
            indices.push_back(diagonal);
        }
    }

    return std::make_shared<Mesh>(vertices, normals, indices, "Sphere");
}

std::shared_ptr<Mesh> Mesh::createPlane() {
    std::vector<glm::vec3> vertices = {
        {-1.0f, 0, 1.0f},
        {1.0f, 0, 1.0f},
        {-1.0f, 0, -1.0f},
        {1.0f, 0, -1.0f},
    };

    std::vector<glm::vec3> normals = {
        {0, 1.0f, 0},
        {0, 1.0f, 0},
        {0, 1.0f, 0},
        {0, 1.0f, 0},
    };

    std::vector<unsigned int> indices = {
        0, 1, 2, 3, 2, 1};

    return std::make_shared<Mesh>(vertices, normals, indices, "Plane");
}

std::shared_ptr<Mesh> Mesh::createBox() {
    // cube but with faces toward inside

    std::vector<glm::vec3> vertices = {
        // Arrière
        {-1.0f, -1.0f, -1.0f},
        {1.0f, -1.0f, -1.0f},
        {1.0f, 1.0f, -1.0f},
        {-1.0f, 1.0f, -1.0f},
        // Avant
        {-1.0f, -1.0f, 1.0f},
        {1.0f, -1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f},
        // Gauche
        {-1.0f, -1.0f, -1.0f},
        {-1.0f, 1.0f, -1.0f},
        {-1.0f, 1.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f},
        // Droite
        {1.0f, -1.0f, -1.0f},
        {1.0f, 1.0f, -1.0f},
        {1.0f, 1.0f, 1.0f},
        {1.0f, -1.0f, 1.0f},
        // Bas
        {-1.0f, -1.0f, -1.0f},
        {1.0f, -1.0f, -1.0f},
        {1.0f, -1.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f},
        // Haut
        {-1.0f, 1.0f, -1.0f},
        {1.0f, 1.0f, -1.0f},
        {1.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f}};

    std::vector<glm::vec3> normals = {
        // Normales
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f}, // Arrière
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f}, // Avant
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f}, // Gauche
        {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f}, // Droite
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f}, // Bas
        {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f} // Haut
    };

    std::vector<unsigned int> indices = {
        // Face arrière
        1, 2, 0, 3, 0, 2,
        // Face avant
        5, 4, 6, 7, 6, 4,
        // Face gauche
        9, 10, 8, 11, 8, 10,
        // Face droite
        13, 12, 14, 15, 14, 12,
        // Face inférieure
        17, 16, 18, 19, 18, 16,
        // Face supérieure
        21, 22, 20, 23, 20, 22};

    return std::make_shared<Mesh>(vertices, normals, indices, "Box");
}
