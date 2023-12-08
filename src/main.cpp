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
std::vector<float> forceXL;
std::vector<float> forceYL;
std::vector<float> accelXL;
std::vector<float> accelYL;

std::vector<float> velocityXL;
std::vector<float> velocityYL;

std::vector<float> oldPositionXL;
std::vector<float> oldPositionYL;
std::vector<float> bufferPositionXL;
std::vector<float> bufferPositionYL;

std::vector<float> massL;

const float G = 10.0f;
const float TIMESTEP = 0.001f;

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
  long unsigned i, j;
  //int r1, r2;
  int x, y;
  float dist, minDist = 0.0f;
  float sumRadii = 2.0f;
  float deltaX, deltaY;

  addParticle(100, 200, 200, 0, 0);
  for (i = 0; i < 100; i++) {
   // r1 = std::rand() % 2 ? 1 : -1;
   // r2 = std::rand() % 2 ? 1 : -1;
    minDist = 0.0f;
    while(minDist < sumRadii) {
        x = (int) (std::rand() % 390) + 10;
        y = (int) (std::rand() % 390) + 10;
        for (j = 0; j < massL.size(); ++j) {
            deltaX = positionXL[j] - x;
            deltaY = positionYL[j] - y;
            dist = sqrt(deltaX * deltaX + deltaY * deltaY);
            if (dist < minDist || j == 0) minDist = dist;
        }
     //   std::cout << minDist << std::endl;
    }
    addParticle(100, x, y, 0, 0); //(std::rand() % 20 + 2) * r1, (std::rand() % 20 + 2)  * r2);
    //addParticle(1000, i + 10, i + 10, 0, 0); //(std::rand() % 20 + 2) * r1, (std::rand() % 20 + 2)  * r2);
  }

//   addParticle(std::rand() % 1000, std::rand() % 390 + 10, std::rand() % 390 + 10, 0, 0);
}

void addParticle(int mass, float x, float y, float vx, float vy) {
    massL.push_back(mass);
    positionXL.push_back(x);
    positionYL.push_back(y);
    oldPositionXL.push_back(x);
    oldPositionYL.push_back(y);
    velocityXL.push_back(vx);
    velocityYL.push_back(vy);
    forceXL.push_back(0);
    forceYL.push_back(0);
    accelXL.push_back(0);
    accelYL.push_back(0);
}


void calculateForce() {
    // // CALCULATE FORCES, ACCELERATIONS, VELOCITIES, POSITIONS
    // float distx, disty, A, netAx, netAy, dir;
    // long unsigned size = massL.size();
    // long unsigned i,j;

    // for(i = 0 ; i < size; ++i) {
    //     netAx = 0;
    //     netAy = 0;
    //     for(j = 0 ; j < size; ++j) {
    //         if (j != i) {
    //             distx = positionXL[j] - positionXL[i]; 
    //             disty = positionYL[j] - positionYL[i];

    //             if (sqrt((distx * distx + disty * disty)) <= 5.0f) {
                 
                 
    //                 continue;
    //             }

    //             dir = atan2(disty, distx);

    //             A = massL[j] / (distx * distx + disty * disty);
    //             netAx += A * cos(dir);
    //             netAy += A * sin(dir);
    //         }
    //     }
    //     accelXL[i] = netAx * G;
    //     accelYL[i] = netAy * G;
    // }
    long unsigned i,j;
    float deltaX, deltaY;
    float distance;
    float directionX, directionY;
    float forceX, forceY;

    for (i = 0; i < massL.size(); ++i) {
        for (j = i + 1; j < massL.size(); ++j) {
            deltaX = positionXL[j] - positionXL[i];
            deltaY = positionYL[j] - positionYL[i];
            distance = sqrt(deltaX * deltaX + deltaY * deltaY);
            directionX = deltaX / distance;
            directionY = deltaY / distance;

            // Newton's law of gravity

            if (distance < 2.0f) {
                // forceX = 100 * (2.0f - distance) * (deltaX / distance);
                // forceY = 100 * (2.0f - distance) * (deltaY / distance);
            } else {
            }

                forceX = (directionX * G * massL[i] * massL[j]) / (distance * distance);
                forceY = (directionY * G * massL[i] * massL[j]) / (distance * distance);

            forceXL[i] += forceX;
            forceYL[i] += forceY;
            forceXL[j] -= forceX;
            forceYL[j] -= forceY;   
            
            // F = ma together with v' = a

            // particles[i].velocity += force * dt / particles[i].mass;
            // particles[j].velocity -= force * dt / particles[j].mass;
        }
    }
}

void calculatePosition() {
    long unsigned i;
    for (i = 0; i < massL.size(); ++i) {

        // positionXL[i] += (positionXL[i] - oldPositionXL[i]) + forceXL[i] * TIMESTEP * TIMESTEP / massL[i];
        // positionYL[i] += (positionYL[i] - oldPositionYL[i]) + forceYL[i] * TIMESTEP * TIMESTEP / massL[i];
        // velocityXL[i] = (positionXL[i] - oldPositionXL[i]) / TIMESTEP;
        // velocityYL[i] = (positionYL[i] - oldPositionYL[i]) / TIMESTEP;
        // oldPositionXL[i] = positionXL[i];        
        // oldPositionYL[i] = positionYL[i];
        velocityXL[i] += (forceXL[i] * TIMESTEP) / massL[i];
        velocityYL[i] += (forceYL[i] * TIMESTEP) / massL[i];
        positionXL[i] += velocityXL[i] * TIMESTEP;
        positionYL[i] += velocityYL[i] * TIMESTEP;
    }
}

void calculateCollision() {
    long unsigned i, j;
    float deltaX, deltaY;
    float sumRadii = 10.0f;
    float distance;
    float collisionNormalX, collisionNormalY;
    float relativeSpeedX, relativeSpeedY;
    float constraintSpeed, constraintValue;
    float offsetX, offsetY;
    float impulseX, impulseY;
    float sumMass, reducedMass;
    float elasticity = 0.25;

    // After applying forces and integrating position
    for (i = 0; i < massL.size(); ++i)
    {
        for (j = i + 1; j < massL.size(); ++j)
        {
            deltaX = positionXL[j] - positionXL[i];
            deltaY = positionYL[j] - positionYL[i];
            
            // sumRadii = 2; //particles[j].radius + particles[i].radius;
            distance = sqrt(deltaX * deltaX + deltaY * deltaY);
            collisionNormalX = deltaX / distance;
            collisionNormalY = deltaY / distance;
            relativeSpeedX = velocityXL[j] - velocityXL[i];
            relativeSpeedY = velocityYL[j] - velocityYL[i];
            constraintSpeed = collisionNormalX * relativeSpeedX + collisionNormalY * relativeSpeedY;
            constraintValue = distance - sumRadii; 
                //std::cout << constraintValue << " " << constraintSpeed << " " << (constraintValue < 0 ) << std::endl;
            if (constraintValue < 0.f && constraintSpeed < 0.f)
            {

                std::cout << "It urns " << constraintSpeed << std::endl;
                std::cout << "It urns " << offsetX << " " << offsetY <<  std::endl;
                sumMass = massL[i] + massL[j];
                reducedMass = 1.f / (1.f / massL[i] + 1.f / massL[j]);
                
                offsetX = constraintValue * collisionNormalX;
                offsetY = constraintValue * collisionNormalY;

                positionXL[i] += offsetX * massL[j] / sumMass;
                positionYL[i] += offsetY * massL[j] / sumMass;
                positionXL[j] -= offsetX * massL[i] / sumMass;
                positionYL[j] -= offsetY * massL[i] / sumMass;

                impulseX = collisionNormalX * (-constraintSpeed * (1.f + elasticity)) * reducedMass;
                impulseY = collisionNormalY * (-constraintSpeed * (1.f + elasticity)) * reducedMass;
                
                std::cout << "imp " << impulseX << " " << impulseY <<  std::endl;
                
                velocityXL[i] -= impulseX / massL[i];
                velocityYL[i] -= impulseY / massL[i];
                velocityXL[j] += impulseX / massL[j];
                velocityYL[j] += impulseY / massL[j];
            }
        }
    }
}

void updateWorld() {

    unsigned int k;

    calculateForce();
    calculatePosition();
    for (k = 0; k < 1; ++k) calculateCollision();
    



    // // COLLISION AND BOUNCE
    // for(i = 0 ; i < size; ++i) {
    //     velocityfXL[i] = 0;
    //     velocityfYL[i] = 0;

    //     if (collision[i].size() != 0) {
    //         // for (j = 0; j < collision[i].size(); ++j) {
    //         //     k = collision[i][j];
    //         //     velocityfXL[i] += (massL[k] * (COEFFICIENT_OF_RESTITUTION * (velocityiXL[k] - velocityiXL[i]) + velocityiXL[k]) + massL[i] * velocityiXL[i]) / (massL[k] + massL[i]);
    //         //     velocityfYL[i] += (massL[k] * (COEFFICIENT_OF_RESTITUTION * (velocityiYL[k] - velocityiYL[i]) + velocityiYL[k]) + massL[i] * velocityiYL[i]) / (massL[k] + massL[i]);
    //         //     // velocityfXL[i] = ((massL[i] - massL[k]) * velocityiXL[i] / (massL[i] + massL[k])) + ((2*massL[k]*velocityiXL[k]) / (massL[i] + massL[k]));

    //         //   //  std::cout << i << " collide with " << k << " " << velocityfXL[i] << std::endl;

    //         // }
    //         // collision[i].clear();
    //     } else {
    //         velocityfXL[i] = velocityiXL[i] + accelXL[i] * TIMESTEP; // Not sure if this improves speed in shader since it avoids branching if else
    //         velocityfYL[i] = velocityiYL[i] + accelYL[i] * TIMESTEP;
    //     }
    //     positionXL[i] += velocityfXL[i] * TIMESTEP;
    //     positionYL[i] += velocityfYL[i] * TIMESTEP;
        
    // }

    // std::swap( velocityfXL, velocityiXL );
    // std::swap( velocityfYL, velocityiYL );
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