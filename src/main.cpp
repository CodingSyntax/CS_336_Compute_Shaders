#include "config.h"
#include <math.h>
#include "util.h"

extern char _binary_src_shaders_fragment_frag_start[];
extern char _binary_src_shaders_vertex_vert_start[];

int main(int argc, char *argv[]) {
    std::cout << "Starting..." << std::endl;
    
    char *fragSrc = _binary_src_shaders_fragment_frag_start;
    char *vertSrc = _binary_src_shaders_vertex_vert_start;
    // std::cout << "Frag:\n" << p << std::endl;
    
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
    
    std::vector<float> verts = {
        -1, -1, 1, 0, 0,
         1, -1, 0, 1, 0,
        -1,  1, 0, 0, 1,
         1,  1, 1, 1, 0,
         1, -1, 0, 1, 0,
        -1,  1, 0, 0, 1
    };
    
    unsigned int shader = createShaderProgram(vertSrc, fragSrc);
    
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
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
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