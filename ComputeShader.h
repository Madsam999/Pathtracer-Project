//
// Created by Samuel on 2025-08-19.
//

#ifndef COMPUTESHADER_H
#define COMPUTESHADER_H
#include <string>
#include <glad/glad.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>


class ComputeShader {
public:
    ComputeShader() {
        program = 0;
    }
    ComputeShader(const char* filename) {
        std::string code = readFile(filename);
        GLuint shader = compileShader(code);

        if (shader == 0) {
            throw std::runtime_error("Error: Couldn't compile compute shader: " + std::string(filename));
        }

        program = glCreateProgram();
        glAttachShader(program, shader);
        glLinkProgram(program);
        glDeleteShader(shader);
    }
    void use();
    void dispatch(int x, int y, int z, GLbitfield bitfield);
    void deleteProgram();
    void setFloat4(std::string name, glm::vec4 value);
    void setInt(std::string name, int value);
    void setFloat3(std::string name, glm::vec3 value);
    void setFloat44(std::string name, glm::mat4 value);
    void setBool(std::string name, bool value);
private:
    std::string readFile(const char* filename);
    GLuint compileShader(std::string& shaderSource);
    GLuint program;
};


#endif //COMPUTESHADER_H
