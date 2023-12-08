#include "config.h"
#include <math.h>
#include "util.h"
#include <cstring>
#include <chrono>
#include <thread>
#include <cmath>

#include "dataHandle.h"
#define GLT_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "gltext.h"
#pragma GCC diagnostic pop

#define WG_SIZE_X 8
#define WG_SIZE_Y 8

extern char _binary_src_shaders_fragment_frag_start[];
extern char _binary_src_shaders_vertex_vert_start[];
extern char _binary_src_shaders_gravity_comp_start[];
extern char _binary_src_shaders_color_comp_start[];

const int WIDTH = 400;
const int HEIGHT = 400;

float massSel = 10;

bool pause = true;

GLTtext *massTxt;
GLFWwindow* window;
unsigned int shaderProgram;
unsigned int computeShader;

unsigned int colorBuffer;

unsigned int accelBuffer, particleBuffer;


// VERTICES DATA
unsigned int verticesBuffer;
std::vector<float> verts;
unsigned int texCoordsBuffer;

// TEXTURE DATA
GLuint textureID;
GLubyte* pixels;
GLint textureSamplerLocation;

// UNIVERSE DATA
Data3 particleData;
Data2 accelData;
std::vector<float> velocityXL;
std::vector<float> velocityYL;
std::vector<unsigned long int[2]> combination;

const float G = 10;
const float TIMESTEP = 0.02;

//std::chrono::milliseconds timespan ((int) (0.02 * 1000));

// UNIVERSE FUNCTIONS (USED IN MAIN, DRAW, AND UPDATE)

void initWorld();
void updateWorld();
void addParticle(float mass, float x, float y, float vx, float vy);
void renderParticle();

void pauseSim() {
    pause = !pause;
}

void createMass(GLFWwindow* window) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    double thisX = (x/width) * WIDTH;
    double thisY = HEIGHT - ((y/height) * HEIGHT);
    addParticle(massSel, thisX, thisY, 0, 0);
    std::cout << "(" << thisX << "," << thisY << ")" << std::endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        pauseSim();
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
        createMass(window); //TODO: change to right click, and allow left click to pan the screen?
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    massSel += yoffset;
}

void drawGravSim() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    
    unsigned int pos = glGetAttribLocation(shaderProgram, "a_Position");

    glEnableVertexAttribArray(pos);
    
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glVertexAttribPointer(pos, 2, GL_FLOAT, false, sizeof(float) * 3, 0);
    glUniform2f(glGetUniformLocation(shaderProgram, "scale"), WIDTH, HEIGHT);
    
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    
    glDrawArrays(GL_POINTS, 0, particleData.getSize());
    
    glDisableVertexAttribArray(pos);
    glUseProgram(GL_NONE);
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
    
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    
    textureSamplerLocation = glGetUniformLocation(shaderProgram, "textureSampler");
    glUniform1i(textureSamplerLocation, 0);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(pos);
    glDisableVertexAttribArray(tex);
    
    glUseProgram(GL_NONE);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void computeGravity() {
    glUseProgram(computeShader);
    const int WGX = 64;
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particleData.getFullSize(), particleData.getData(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, accelBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, accelData.getFullSize(), accelData.getData(), GL_DYNAMIC_READ);
    //ceiling math
    glDispatchCompute((particleData.getSize() + WGX - 1) / WGX, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, accelBuffer);
    void *p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
    
    memcpy(accelData.getData(), p, accelData.getFullSize());
    
    glUseProgram(GL_NONE);
}

void drawMassText() {
    glFrontFace(GL_CCW);
    std::ostringstream strStream;
    strStream << "Gravity Demo - Mass: " << massSel;
    glfwSetWindowTitle(window, strStream.str().c_str());
    // gltSetText(massTxt, strStream.str().c_str());
    
    // gltBeginDraw();
    
    // gltColor(1.0f, 1.0f, 1.0f, 1.0f);
    // // gltDrawText2D(massTxt, 0.0f, 0.0f, 1.0f);
    // // float view[] = 
    // // {1, 0, 0, 0,
    // //  0, 1, 0, 0,
    // //  0, 0, 1, 0,
    // //  0, 0, 0, 1};
    // // gltDrawText3D(massTxt, 0, 0, -1, 1, view, view);
    // // gltDrawText(massTxt, view);
    // gltDrawText2DAligned(massTxt, 0, 0, 1, 0, 0);
    
    // gltEndDraw();
}

void draw() {
    if (!pause) computeGravity();
    drawGravSim();
    drawMassText();
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}

int main(int argc, char *argv[]) {
    std::cout << "Starting... " << std::endl;
    
    char *fragSrc = _binary_src_shaders_fragment_frag_start;
    char *vertSrc = _binary_src_shaders_vertex_vert_start;
    char *computeSrc = _binary_src_shaders_gravity_comp_start;
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
    
    if (!gltInit()) {
        glfwTerminate();
        std::cerr << "GLT faild to load" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    massTxt = gltCreateText();
    gltSetText(massTxt, "Hello");
    
    std::cout << "Open GL: " << glGetString(GL_VERSION) << std::endl;
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    
    glGenTextures(1, &colorBuffer);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, WIDTH, HEIGHT);
    
    glGenBuffers(1, &accelBuffer);
    glGenBuffers(1, &particleBuffer);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particleData.getFullSize(), particleData.getData(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particleBuffer);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, accelBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, accelData.getFullSize(), accelData.getData(), GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, accelBuffer);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    initWorld();
    
    shaderProgram = createShaderProgram(vertSrc, fragSrc);
    computeShader = createShaderProgram(computeSrc);
    verticesBuffer = createAndLoadBuffer(verts);
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glBufferData(GL_ARRAY_BUFFER, particleData.getFullSize(), particleData.getData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

  
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glPointSize(3);
    //draw
    
    
    while (!glfwWindowShouldClose(window)) {
        //std::cout << positionXL[0] << std::endl;
        draw();
        if (!pause) updateWorld();
        //std::this_thread::sleep_for(timespan);
    }
    
    gltDeleteText(massTxt);
    gltTerminate();
    glfwDestroyWindow(window);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    
    std::cout << "Exit Success" << std::endl;
    exit(EXIT_SUCCESS);
}

// UNIVERSE FUNCTIONS IMPLEMENTATION
void initWorld() {
    long unsigned i, j;
    //int r1, r2;
    int x, y;
    float dist, minDist = 0.0f;
    float sumRadii = 2.0f;
    float deltaX, deltaY;
    addParticle(100, 200, 200, 0, 0);
    for (i = 0; i < 20000; i++) {
    // r1 = std::rand() % 2 ? 1 : -1;
    // r2 = std::rand() % 2 ? 1 : -1;
        minDist = 0.0f;
        while(minDist < sumRadii) {
            x = (int) (std::rand() % 390) + 10;
            y = (int) (std::rand() % 390) + 10;
            for (j = 0; j < particleData.getSize(); ++j) {
                deltaX = particleData[j][0] - x;
                deltaY = particleData[j][1] - y;
                dist = sqrt(deltaX * deltaX + deltaY * deltaY);
                if (dist < minDist || j == 0) minDist = dist;
            }
        //   std::cout << minDist << std::endl;
        }
        addParticle(100, x, y, 0, 0);
    }
    
    // addParticle(10, 10, 10, 0, 0);
    // addParticle(10, 50, 50, 0, 0);
    // addParticle(10000, 100, 100, 0, 0);
    // addParticle(12, 200, 200, 0, 0);
    // addParticle(12, 300, 300, 0, 0);
    // addParticle(12, 10, 300, 0, 0);
    // addParticle(10, 0, 0, 0, 0);
    // addParticle(10, 400, 400, 0, 0);
    // addParticle(10, 100, 100, 0, 0);
    // addParticle(10, 200, 200, 0, 0);
    // addParticle(10, 300, 300, 0, 0);
    // addParticle(10, 10, 10, 0, 0);
}

void addParticle(float mass, float x, float y, float vx, float vy) {
    float data[3] = {x, y, mass};
    particleData.add(data);
    verts.push_back(x/WIDTH);
    verts.push_back(y/HEIGHT);
    
    // massL.push_back(mass);
    // positionXL.push_back(x);
    // positionYL.push_back(y);
    velocityXL.push_back(vx);
    velocityYL.push_back(vy);
    float accel[2] = {0,0};
    accelData.add(accel);
    // accelXL.push_back(0);
    // accelYL.push_back(0);
}

void updateWorld() {
    // CALCULATE VELOCITIES, POSITIONS
    long unsigned size = particleData.getSize();
    long unsigned i;
    for(i = 0 ; i < size; ++i) {
        velocityXL[i] += accelData[i][0] * TIMESTEP;
        velocityYL[i] += accelData[i][1] * TIMESTEP;
        particleData[i][0] += velocityXL[i] * TIMESTEP;
        particleData[i][1] += velocityYL[i] * TIMESTEP;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glBufferData(GL_ARRAY_BUFFER, particleData.getFullSize(), particleData.getData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // for (int i = 0; i < particleData.getSize(); i++) {
    //     std::cout << "(" << particleData[i][0] << "," << particleData[i][1] << ")";
    // }
    // std::cout << std::endl;
}