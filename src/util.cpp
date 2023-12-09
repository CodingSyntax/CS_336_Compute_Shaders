#include "util.h"

unsigned int loadAndCompileShader(const char *shaderSrc, unsigned int moduleType, std::string name) {    
    unsigned int shaderModule = glCreateShader(moduleType);
    glShaderSource(shaderModule, 1, &shaderSrc, NULL);
    glCompileShader(shaderModule);
    
    int success;
    glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);

    if (!success) {
        char errorLog[1024];
        glGetShaderInfoLog(shaderModule, 1024, NULL, errorLog);
        std::cerr << "Shader Module compilation error: (" << name << ")\n" << errorLog << std::endl;
    }
    
    return shaderModule;
}

unsigned int loadAndCompileShader(const char *shaderSrc, unsigned int moduleType) {    
    return loadAndCompileShader(shaderSrc, moduleType, "unknown");
}

unsigned int createShaderProgram(const char *vertexSrc, const char *fragmentSrc, std::string vName, std::string fName) {
    std::vector<unsigned int> modules;
    modules.push_back(loadAndCompileShader(vertexSrc, GL_VERTEX_SHADER, vName));
    modules.push_back(loadAndCompileShader(fragmentSrc, GL_FRAGMENT_SHADER, fName));
    
    unsigned int shader = glCreateProgram();
    for (unsigned int shaderModule : modules) {
        glAttachShader(shader, shaderModule);
    }
    
    glLinkProgram(shader);
    
    int success;
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
        char errorLog[1024];
        glGetShaderInfoLog(shader, 1024, NULL, errorLog);
        std::cerr << "Shader linking error: (" << vName << "," << fName << ")\n" << errorLog << std::endl;
    }
    
    for (unsigned int shaderModule : modules) {
        glDeleteShader(shaderModule);
    }
    
    return shader;
}

unsigned int createShaderProgram(const char *vertexSrc, const char *fragmentSrc) {
    return createShaderProgram(vertexSrc, fragmentSrc, "unknown", "unknown");
}

unsigned int createShaderProgram(const char *computeSrc, std::string name) {
    unsigned int module = loadAndCompileShader(computeSrc, GL_COMPUTE_SHADER, name);
    
    unsigned int shader = glCreateProgram();
    glAttachShader(shader, module);
    glLinkProgram(shader);
    
    int success;
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
        char errorLog[1024];
        glGetShaderInfoLog(shader, 1024, NULL, errorLog);
        std::cerr << "Shader linking error: (" << name << ")\n" << errorLog << std::endl;
    }
    
    glDeleteShader(module);
    
    return shader;
}

unsigned int createShaderProgram(const char *computeSrc) {
    return createShaderProgram(computeSrc, "unknown");
}

unsigned int createAndLoadIndexBuffer(std::vector<unsigned int> data) {
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned int), data.data(), GL_STATIC_DRAW);
    return buffer;
}

//TODO: complete if needed
unsigned int createAndLoadTexture(void *texture) {
    unsigned int textureHandle;
    glGenTextures(1, &textureHandle);
    
    glActiveTexture(GL_TEXTURE0);
    
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    return -1;
    //glPixelStorei(GL_UNPACK)
}

