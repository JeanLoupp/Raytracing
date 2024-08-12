#include "ShaderProgram.hpp"

ShaderProgram::ShaderProgram(const std::string &vertexPath, const std::string &fragmentPath) {
    std::string vertexCode = loadShaderSource(vertexPath);
    std::string fragmentCode = loadShaderSource(fragmentPath);

    unsigned int vertexShader = compileShader(vertexCode, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentCode, GL_FRAGMENT_SHADER);

    programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    glLinkProgram(programID);

    int success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programID, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

ShaderProgram::~ShaderProgram() {
    glDeleteProgram(programID);
}

std::string ShaderProgram::loadShaderSource(const std::string &filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int ShaderProgram::compileShader(const std::string &source, GLenum type) {
    unsigned int shader = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    return shader;
}