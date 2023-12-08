#include "config.h"
#include <math.h>
#include "util.h"
#include <cstring>
#include <chrono>
#include <thread>
#include <cmath>

extern char _binary_src_shaders_fragment_frag_start[];
extern char _binary_src_shaders_vertex_vert_start[];

const int WIDTH = 400;
const int HEIGHT = 400;

GLFWwindow* window;
unsigned int shaderProgram;

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

std::vector<float> velocityiXL;
std::vector<float> velocityiYL;
std::vector<float> velocityfXL;
std::vector<float> velocityfYL;

std::vector<float> massL;
std::vector<std::vector<int>> collision;

const float G = 0.2;
const float TIMESTEP = 0.02;
const float COEFFICIENT_OF_RESTITUTION = 0.55f;

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


void draw() {
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

    glEnd();
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}

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
    
    std::vector<float> verts = {
        -1, 1, 1, 1, -1, -1,
        -1, -1, 1, 1, 1, -1
    };


    std::vector<float> texCoords = {
        0, 1, 1, 1, 0, 0,
        0, 0, 1, 1, 1, 0
    };
    
    shaderProgram = createShaderProgram(vertSrc, fragSrc);
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

  long unsigned i;
  //int r1, r2;
  addParticle(1000, 200, 200, 0, 0);
  for (i = 0; i < 500; i++) {
   // r1 = std::rand() % 2 ? 1 : -1;
   // r2 = std::rand() % 2 ? 1 : -1;
    addParticle(std::rand() % 1000, std::rand() % 390 + 10, std::rand() % 390 + 10, 0, 0); //(std::rand() % 20 + 2) * r1, (std::rand() % 20 + 2)  * r2);
  }
    
    // addParticle(100, 100, 210, 5, -5);
    // addParticle(100, 170, 210, 0, -5);
    // addParticle(100, 240, 210, -5, -5);
    //addParticle(100, 215, 218.66025f, 0, 0);
    // addParticle(200, 50, 50, 0, 0);
    // addParticle(100, 100, 50, 0, 0);
    // addParticle(120, 200, 200, 0, 0);
    // addParticle(120, 300, 300, 0, 0);
    // addParticle(120, 10, 300, 0, 0);
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
    velocityiXL.push_back(vx);
    velocityiYL.push_back(vy);
    velocityfXL.push_back(0);
    velocityfYL.push_back(0);
    accelXL.push_back(0);
    accelYL.push_back(0);
    collision.push_back({});
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

            if (abs(distx) <= 1 && abs(disty) <= 1) {
                //collision[i].push_back(j);
                continue;
            }

            dir = atan2(disty, distx);

            A = massL[j] / (distx * distx + disty * disty);
            netAx += A * cos(dir);
            netAy += A * sin(dir);
        }
        accelXL[i] = netAx * G;
        accelYL[i] = netAy * G;

    }
  //  unsigned long int k;

    // COLLISION AND BOUNCE
    for(i = 0 ; i < size; ++i) {
        velocityfXL[i] = 0;
        velocityfYL[i] = 0;

        if (collision[i].size() != 0) {
            // for (j = 0; j < collision[i].size(); ++j) {
            //     k = collision[i][j];
            //     velocityfXL[i] += (massL[k] * (COEFFICIENT_OF_RESTITUTION * (velocityiXL[k] - velocityiXL[i]) + velocityiXL[k]) + massL[i] * velocityiXL[i]) / (massL[k] + massL[i]);
            //     velocityfYL[i] += (massL[k] * (COEFFICIENT_OF_RESTITUTION * (velocityiYL[k] - velocityiYL[i]) + velocityiYL[k]) + massL[i] * velocityiYL[i]) / (massL[k] + massL[i]);
            //     // velocityfXL[i] = ((massL[i] - massL[k]) * velocityiXL[i] / (massL[i] + massL[k])) + ((2*massL[k]*velocityiXL[k]) / (massL[i] + massL[k]));

            //   //  std::cout << i << " collide with " << k << " " << velocityfXL[i] << std::endl;

            // }
            // collision[i].clear();
        } else {
            velocityfXL[i] = velocityiXL[i] + accelXL[i] * TIMESTEP; // Not sure if this improves speed in shader since it avoids branching if else
            velocityfYL[i] = velocityiYL[i] + accelYL[i] * TIMESTEP;
        }
        positionXL[i] += velocityfXL[i] * TIMESTEP;
        positionYL[i] += velocityfYL[i] * TIMESTEP;
        
    }

    std::swap( velocityfXL, velocityiXL );
    std::swap( velocityfYL, velocityiYL );
    
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
        
        pixels[(x + y * WIDTH) * 4 + 0] = i;  // Red
        pixels[(x + y * WIDTH) * 4 + 1] = 255;  // Green
        pixels[(x + y * WIDTH) * 4 + 2] = 255;  // Blue
        pixels[(x + y * WIDTH) * 4 + 3] = 255;  // Alpha
    
    }
}