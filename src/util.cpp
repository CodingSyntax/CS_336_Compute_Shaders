#include "util.h"

unsigned int loadAndCompileShader(const std::string &filePath, unsigned int moduleType) {
    std::ifstream file;
    std::stringstream bufferedLines;
    std::string line;
    
    file.open(filePath);
    while (std::getline(file, line)) {
        bufferedLines << line << "\n";
    }
    
    std::string shaderSource = bufferedLines.str();
    const char* shaderSrc = shaderSource.c_str();
    bufferedLines.str("");
    file.close();
    
    unsigned int shaderModule = glCreateShader(moduleType);
    glShaderSource(shaderModule, 1, &shaderSrc, NULL);
    glCompileShader(shaderModule);
    
    int success;
    glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);
    if (!success) {
        char errorLog[1024];
        glGetShaderInfoLog(shaderModule, 1024, NULL, errorLog);
        std::cerr << "Shader Module compilation error:\n" << errorLog << std::endl;
    }
    
    return shaderModule;
}

unsigned int createShaderProgram(const std::string &vertexFilePath, const std::string &fragmentFilePath) {
    std::vector<unsigned int> modules;
    modules.push_back(loadAndCompileShader(vertexFilePath, GL_VERTEX_SHADER));
    modules.push_back(loadAndCompileShader(fragmentFilePath, GL_FRAGMENT_SHADER));
    
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
        std::cerr << "Shader linking error:\n" << errorLog << std::endl;
    }
    
    for (unsigned int shaderModule : modules) {
        glDeleteShader(shaderModule);
    }
    
    return shader;
}

unsigned int createAndLoadIndexBuffer(std::vector<unsigned int> data) {
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned int), data.data(), GL_STATIC_DRAW);
    return buffer;
}