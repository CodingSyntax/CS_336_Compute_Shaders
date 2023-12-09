#pragma once
#include "config.h"
#include "util.h"

unsigned int loadAndCompileShader(const char *shaderSrc, unsigned int moduleType);

unsigned int createShaderProgram(const char *vertexSrc, const char *fragmentSrc, std::string vName, std::string fName);
unsigned int createShaderProgram(const char *vertexSrc, const char *fragmentSrc);
unsigned int createShaderProgram(const char *computeSrc, std::string name);
unsigned int createShaderProgram(const char *computeSrc);

template <typename T>
unsigned int createAndLoadBuffer(std::vector<T> data) {
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
    return buffer;
}

unsigned int createAndLoadIndexBuffer(std::vector<unsigned int> data);