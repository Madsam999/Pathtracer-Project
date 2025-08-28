//
// Created by Samuel on 2025-08-19.
//

#ifndef SHADER_H
#define SHADER_H
#include <string>
#include <glad/glad.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>


class Shader {
public:
    Shader() {
        program = 0;
    }
    Shader(const char* vertSource, const char* fragSource) {
        std::string vertShaderSource = readFile(vertSource);
        std::string fragShaderSource = readFile(fragSource);

        GLuint vertShader = compileShader(GL_VERTEX_SHADER, vertShaderSource);
        GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, fragShaderSource);

        if (vertShader == GL_NONE || fragShader == GL_NONE) {
            throw std::runtime_error("Error: Couldn't compile shader: " + std::string(vertShaderSource));
        }

        program = glCreateProgram();
        glAttachShader(program, vertShader);
        glAttachShader(program, fragShader);
        glLinkProgram(program);
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
    }
    void use();
    void deleteProgram();
    GLuint getProgram() { return program; }

    void setInt(std::string name, int value);
private:
    std::string readFile(const char* fileName);
    GLuint compileShader(GLenum shaderType, std::string& shaderSource);
    GLuint program;
};



#endif //SHADER_H
