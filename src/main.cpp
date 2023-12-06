#include "config.h"
#include <math.h>
// #include <GL/glcorearb.h>
// #include <GL/glext.h>


unsigned int makeModule(const std::string &filePath, unsigned int moduleType) {
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

unsigned int makeShader(const std::string &vertexFilePath, const std::string &fragmentFilePath) {
    std::vector<unsigned int> modules;
    modules.push_back(makeModule(vertexFilePath, GL_VERTEX_SHADER));
    modules.push_back(makeModule(fragmentFilePath, GL_FRAGMENT_SHADER));
    
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

void drawCircle() {
    glColor3f(0.2, 0.7, 0.2);
    glBegin(GL_POLYGON);
    float radius = 0.25;
    for (int i =0; i<360;i++){
        float degRad = i * (M_PI / 180);
        glVertex2f(cos(degRad) * radius, sin(degRad) * radius);
    }
    // glVertex3f(0, 0, 0);
    // glVertex3f(1, 0, 0);
    // glVertex3f(0, 1, 0);
    
    glEnd();
}

template <typename T>
unsigned int createAndLoadBuffer(std::vector<T> data) {
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
    // glBindBuffer(GL_ARRAY_BUFFER, NULL);
    return buffer;
}

int main(int argc, char *argv[]) {
    std::cout << "Starting..." << std::endl;
    
    if (!glfwInit()) {
        std::cerr << "Failed to init" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    
    GLFWwindow* window = glfwCreateWindow(400, 400, "Project", NULL, NULL);
    
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create window" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        std::cerr << "GLAD faild to load" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    //Guarantees viewport = framebuffer size
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);
    
    std::vector<float> verts = {
        -1, -1, 1, 0, 0,
         1, -1, 0, 1, 0,
         0,  1, 0, 0, 1
    };
    
    unsigned int shader = makeShader("src/shaders/vertex.vert", "src/shaders/fragment.frag");
    
    unsigned int vBuffer = createAndLoadBuffer(verts);
    
    glClearColor(0.2f, 0.8f, 0.8f, 1.0f);
    
    //draw
    while (!glfwWindowShouldClose(window)) {
        //Setup view
        // float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        
        // ratio = width / (float)height;
        glViewport(0, 0, width, height);
        
        glClear(GL_COLOR_BUFFER_BIT);
        //Drawing
        glUseProgram(shader);
        
        glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
        
        unsigned int pos, color;
        
        pos = glGetAttribLocation(shader, "a_Position");
        color = glGetAttribLocation(shader, "a_Color");
        
        glEnableVertexAttribArray(pos);
        glEnableVertexAttribArray(color);
        
        glVertexAttribPointer(pos, 2, GL_FLOAT, false, 5*4, 0);
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wpointer-arith"
        glVertexAttribPointer(color, 3, GL_FLOAT, false, 5*4, (void *)0+2*4);
        #pragma GCC diagnostic pop
        
        // glBindBuffer(GL_ARRAY_BUFFER, NULL);
        
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glDisableVertexAttribArray(pos);
        glDisableVertexAttribArray(color);
        // glUseProgram(NULL);
        
        glEnd();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    glDeleteProgram(shader);
    glfwTerminate();
    
    std::cout << "Exit Success" << std::endl;
    exit(EXIT_SUCCESS);
}