#include "config.h"
#include <math.h>
#include "util.h"
#include <cstring>
#include <chrono>
#include <thread>
#include <cmath>

#define WG_SIZE_X 8
#define WG_SIZE_Y 8

extern char _binary_src_shaders_fragment_frag_start[];
extern char _binary_src_shaders_vertex_vert_start[];
extern char _binary_src_shaders_gravity_comp_start[];
extern char _binary_src_shaders_color_comp_start[];

const int WIDTH = 400;
const int HEIGHT = 400;

GLFWwindow* window;
unsigned int shaderProgram;
unsigned int computeShader;

unsigned int colorBuffer;

// VERTICES DATA
unsigned int verticesBuffer;
unsigned int texCoordsBuffer;

// TEXTURE DATA
GLuint textureID;
GLubyte* pixels;
GLint textureSamplerLocation;

// UNIVERSE DATA
std::vector<float> positionXL;
std::vector<float> positionYL;
std::vector<float> accelXL;
std::vector<float> accelYL;
std::vector<float> velocityXL;
std::vector<float> velocityYL;
std::vector<int> massL;
std::vector<unsigned long int[2]> combination;

const float G = 0.1;
const float TIMESTEP = 0.02;

//std::chrono::milliseconds timespan ((int) (0.02 * 1000));

// UNIVERSE FUNCTIONS (USED IN MAIN, DRAW, AND UPDATE)

void initWorld();
void updateWorld();
void addParticle(int mass, float x, float y, float vx, float vy);
void renderParticle();

void initTexture() {
    glEnable(GL_TEXTURE_2D);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Allocate memory for pixel data
    pixels = new GLubyte[WIDTH * HEIGHT * 4];
    memset(pixels, 0, WIDTH * HEIGHT * 4);
}

void updateTexture() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
    textureSamplerLocation = glGetUniformLocation(shaderProgram, "textureSampler");
    glUniform1i(textureSamplerLocation, 0);
}

void drawGravSim() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    
    unsigned int pos, tex;
    pos = glGetAttribLocation(shaderProgram, "a_Position");
    tex = glGetAttribLocation(shaderProgram, "a_TexCoords");

    glEnableVertexAttribArray(pos);
    glEnableVertexAttribArray(tex);
    
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glVertexAttribPointer(pos, 2, GL_FLOAT, false, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, texCoordsBuffer);
    glVertexAttribPointer(tex, 2, GL_FLOAT, false, 0, 0);
    
    // glBindBuffer(GL_ARRAY_BUFFER, NULL);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(pos);
    glDisableVertexAttribArray(tex);
    // glUseProgram(NULL);

    renderParticle();
    updateTexture();
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void drawComputeShader() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(computeShader);
    
    
    
    glBindImageTexture(0, colorBuffer, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    //floor
    glDispatchCompute((width + WG_SIZE_X - 1) / WG_SIZE_X, (height + WG_SIZE_Y - 1) / WG_SIZE_Y, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    
    glUseProgram(shaderProgram);
    
    unsigned int pos, tex;
    pos = glGetAttribLocation(shaderProgram, "a_Position");
    tex = glGetAttribLocation(shaderProgram, "a_TexCoords");

    glEnableVertexAttribArray(pos);
    glEnableVertexAttribArray(tex);
    
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glVertexAttribPointer(pos, 2, GL_FLOAT, false, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, texCoordsBuffer);
    glVertexAttribPointer(tex, 2, GL_FLOAT, false, 0, 0);
    
    // glBindBuffer(GL_ARRAY_BUFFER, NULL);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    
    textureSamplerLocation = glGetUniformLocation(shaderProgram, "textureSampler");
    glUniform1i(textureSamplerLocation, 0);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(pos);
    glDisableVertexAttribArray(tex);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}


void draw() {
    drawComputeShader();
}

int main(int argc, char *argv[]) {
    std::cout << "Starting... " << std::endl;
    
    char *fragSrc = _binary_src_shaders_fragment_frag_start;
    char *vertSrc = _binary_src_shaders_vertex_vert_start;
    char *computeSrc = _binary_src_shaders_color_comp_start;
    // std::cout << "Frag:\n" << p << std::endl;
    
    if (!glfwInit()) {
        std::cerr << "Failed to init" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    
    window = glfwCreateWindow(WIDTH, HEIGHT, "Project", NULL, NULL);
    
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
    
    std::cout << "Open GL: " << glGetString(GL_VERSION) << std::endl;
    
    glGenTextures(1, &colorBuffer);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, WIDTH, HEIGHT);
    
    
    
    std::vector<float> verts = {
        -1, 1, 1, 1, -1, -1,
        -1, -1, 1, 1, 1, -1
    };


    std::vector<float> texCoords = {
        0, 1, 1, 1, 0, 0,
        0, 0, 1, 1, 1, 0
    };
    
    shaderProgram = createShaderProgram(vertSrc, fragSrc);
    computeShader = createShaderProgram(computeSrc);
    verticesBuffer = createAndLoadBuffer(verts);
    texCoordsBuffer = createAndLoadBuffer(texCoords);

    initTexture();
    initWorld();
  
    glClearColor(0.2f, 0.8f, 0.8f, 1.0f);
    
    //draw
    
    
    while (!glfwWindowShouldClose(window)) {
        updateWorld();
        //std::cout << positionXL[0] << std::endl;
        draw();
        //std::this_thread::sleep_for(timespan);
    }
    
    glfwDestroyWindow(window);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    
    std::cout << "Exit Success" << std::endl;
    exit(EXIT_SUCCESS);
}

// UNIVERSE FUNCTIONS IMPLEMENTATION
void initWorld() {
    addParticle(10, 10, 10, 0, 0);
    addParticle(10, 50, 50, 0, 0);
    addParticle(10000, 100, 100, 0, 0);
    addParticle(12, 200, 200, 0, 0);
    addParticle(12, 300, 300, 0, 0);
    addParticle(12, 10, 300, 0, 0);
    // addParticle(10, 0, 0, 0, 0);
    // addParticle(10, 400, 400, 0, 0);
    // addParticle(10, 100, 100, 0, 0);
    // addParticle(10, 200, 200, 0, 0);
    // addParticle(10, 300, 300, 0, 0);
    // addParticle(10, 10, 10, 0, 0);
}

void addParticle(int mass, float x, float y, float vx, float vy) {
    massL.push_back(mass);
    positionXL.push_back(x);
    positionYL.push_back(y);
    velocityXL.push_back(vx);
    velocityYL.push_back(vy);
    accelXL.push_back(0);
    accelYL.push_back(0);
}

void updateWorld() {
    // CALCULATE FORCES, ACCELERATIONS, VELOCITIES, POSITIONS
    float distx, disty, A, netAx, netAy, dir;
    long unsigned size = massL.size();
    long unsigned i,j;

    for(i = 0 ; i < size; ++i) {
        netAx = 0;
        netAy = 0;
        for(j = 0 ; j < size; ++j) {
            if (j == i) continue;

            distx = positionXL[j] - positionXL[i]; 
            disty = positionYL[j] - positionYL[i];

            if (abs(distx) <= 1 && abs(disty) <= 1) continue; //Prevents division by 0
            dir = atan2(disty, distx);

            A = massL[j] / (distx * distx + disty * disty);
            netAx += A * cos(dir);
            netAy += A * sin(dir);
        }
        accelXL[i] = netAx * G;
        accelYL[i] = netAy * G;

    }
    
    for(i = 0 ; i < size; ++i) {
        velocityXL[i] += accelXL[i] * TIMESTEP;
        velocityYL[i] += accelYL[i] * TIMESTEP;
        positionXL[i] += velocityXL[i] * TIMESTEP;
        positionYL[i] += velocityYL[i] * TIMESTEP;
    }
}

void renderParticle() {
    long long unsigned int i;
    int x, y;

    memset(pixels, 0, WIDTH * HEIGHT * 4);
    
    for (i = 0; i < massL.size(); ++i) {
        x = (int)positionXL[i];
        y = (int)positionYL[i];
        if (x >= WIDTH || x < 0 || y >= HEIGHT || y < 0) continue;
      //  std::cout << x << " " << y << std::endl;
        pixels[(x + y * WIDTH) * 4 + 0] = 255;  // Red
        pixels[(x + y * WIDTH) * 4 + 1] = 255;  // Green
        pixels[(x + y * WIDTH) * 4 + 2] = 255;  // Blue
        pixels[(x + y * WIDTH) * 4 + 3] = 255;  // Alpha
    }
}