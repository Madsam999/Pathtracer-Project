//
// Created by Samuel on 2025-08-19.
//

#include "ComputeShader.h"

void ComputeShader::use() {
    glUseProgram(program);
}

void ComputeShader::dispatch(int x, int y, int z, GLbitfield bitfield) {
    glDispatchCompute(x, y, z);
    glMemoryBarrier(bitfield);
}

void ComputeShader::deleteProgram() {
    glDeleteProgram(program);
}


std::string ComputeShader::readFile(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Failed to open file " << filename << std::endl;
        return "";
    }
    std::stringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

GLuint ComputeShader::compileShader(std::string& shaderSource) {
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    const char* source = shaderSource.c_str();
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* infoLog = (char*)malloc(length);
        glGetShaderInfoLog(shader, length, NULL, infoLog);
        printf("%s\n", infoLog);
        free(infoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}


void ComputeShader::setFloat4(std::string name, glm::vec4 value) {
    int location = glGetUniformLocation(program, name.c_str());
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void ComputeShader::setInt(std::string name, int value) {
    int location = glGetUniformLocation(program, name.c_str());
    glUniform1i(location, value);
}

void ComputeShader::setFloat3(std::string name, glm::vec3 value) {
    int location = glGetUniformLocation(program, name.c_str());
    glUniform3f(location, value.x, value.y, value.z);
}

void ComputeShader::setFloat44(std::string name, glm::mat4 value) {
    int location = glGetUniformLocation(program, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void ComputeShader::setBool(std::string name, bool value) {
    int location = glGetUniformLocation(program, name.c_str());
    glUniform1i(location, value);
}