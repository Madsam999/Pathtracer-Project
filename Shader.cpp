//
// Created by Samuel on 2025-08-19.
//

#include "Shader.h"

void Shader::use() {
    glUseProgram(program);
}

void Shader::deleteProgram() {
    glDeleteProgram(program);
}

std::string Shader::readFile(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Failed to open file " << filename << std::endl;
        return "";
    }
    std::stringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

GLuint Shader::compileShader(GLenum shaderType, std::string& source) {
    GLuint id = glCreateShader(shaderType);
    const char* sourceCString = source.c_str();
    glShaderSource(id, 1, &sourceCString, NULL);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> errorMessage(length + 1);
        glGetShaderInfoLog(id, length, &length, &errorMessage[0]);
        std::cout << errorMessage.data() << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

void Shader::setInt(std::string name, int value) {
    glUniform1i(glGetUniformLocation(program, name.c_str()), value);
}

