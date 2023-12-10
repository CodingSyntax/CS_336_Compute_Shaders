#include "config.h"
#include <math.h>
#include "util.h"
#include <cstring>
#include <chrono>
#include <thread>
#include <cmath>
#include <bits/stdc++.h>
#include <algorithm>

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

std::vector<bool> collision;

std::vector<float> massL;

const float G = 10.0f;
const float TIMESTEP = 0.01f;
const float sumRadii = 10.0f;
const float eccentricity = 0.8f;

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
    float deltaX, deltaY;

    addParticle(300, 200, 200, 0, 0);
    // addParticle(100, 150, 286.6025, 0, 0);
    addParticle(300, 100, 200, 0, 0);
    for (i = 0; i < 300; i++) {
    // r1 = std::rand() % 2 ? 1 : -1;
    // r2 = std::rand() % 2 ? 1 : -1;
        minDist = 0.0f;
        while(minDist <= sumRadii) {
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
    }
}

void addParticle(int mass, float x, float y, float vx, float vy) {
    massL.push_back(mass);
    positionXL.push_back(x);
    positionYL.push_back(y);
    collision.push_back(0);
    velocityXL.push_back(vx);
    velocityYL.push_back(vy);
    forceXL.push_back(0);
    forceYL.push_back(0);
    accelXL.push_back(0);
    accelYL.push_back(0);
}


void calculateForce() {
    long unsigned i,j;
    float deltaX, deltaY;
    float distance;
    float directionX, directionY;
    float forceX, forceY;

    for (i = 0; i < massL.size(); ++i) {
        forceXL[i] = 0;
        forceYL[i] = 0;
    }

    for (i = 0; i < massL.size(); ++i) {
        for (j = i + 1; j < massL.size(); ++j) {
            deltaX = positionXL[j] - positionXL[i];
            deltaY = positionYL[j] - positionYL[i];
            distance = sqrt(deltaX * deltaX + deltaY * deltaY);
            directionX = deltaX / distance;
            directionY = deltaY / distance;

            forceX = (directionX * G * massL[i] * massL[j]) / (distance * distance);
            forceY = (directionY * G * massL[i] * massL[j]) / (distance * distance);

            forceXL[i] += forceX;
            forceYL[i] += forceY;
            forceXL[j] -= forceX;
            forceYL[j] -= forceY;   
        }
    }
}

void calculatePosition() {
    long unsigned i;
    for (i = 0; i < massL.size(); ++i) {
        if (!collision[i]) { 
            velocityXL[i] += (forceXL[i] * TIMESTEP) / massL[i]; //+ iVelocityXL[i];
            velocityYL[i] += (forceYL[i] * TIMESTEP) / massL[i]; //+ iVelocityYL[i];
            positionXL[i] += velocityXL[i] * TIMESTEP;
            positionYL[i] += velocityYL[i] * TIMESTEP;
        }
        collision[i] = 0;
    }
}

class CollisionEvent {
    public:
        unsigned long primary, secondary;
        bool isValid;
        double time;
        double b;
        double uDx, uDy;

        CollisionEvent(unsigned long primary, unsigned long secondary, double time, double b, double uDx, double uDy) {
            this->primary = primary;
            this->secondary = secondary;
            this->time = time;
            this->b = b;
            this->uDx = uDx;
            this->uDy = uDy;
            this->isValid = true;
        }

        ~CollisionEvent() {};
};

class CollisionEventComparator {
    public:
        int operator() (const CollisionEvent* p1, const CollisionEvent* p2) {
            return p1->time > p2->time;
        }
};

std::unordered_map<long unsigned int, CollisionEvent *> collisionEventMap; 
std::priority_queue <CollisionEvent *, std::vector<CollisionEvent *>, CollisionEventComparator > collisionPriority;

unsigned long uniquePairingHash(unsigned long a, unsigned long b) {
    unsigned long first = a;
    unsigned long second = b;
    if (a > b) {
        first = b;
        second = a;
    }

    return first + (second * (second - 1)) / 2;
}

void doCollisionDetection(long unsigned int primary, long unsigned int secondary) {
    double t;
    double dist, distX, distY;
    double relativeVelocitySq, relativeVelocityX, relativeVelocityY;
    double b, d;
    double fpvx, fpvy, fsvx, fsvy;

    fpvx = velocityXL[primary] + (forceXL[primary] * TIMESTEP) / massL[primary];
    fpvy = velocityYL[primary] + (forceYL[primary] * TIMESTEP) / massL[primary]; 
    fsvx = velocityXL[secondary] + (forceXL[secondary] * TIMESTEP) / massL[secondary];
    fsvy = velocityYL[secondary] + (forceYL[secondary] * TIMESTEP) / massL[secondary]; 

    distX = positionXL[secondary] - positionXL[primary];
    distY = positionYL[secondary] - positionYL[primary];
    relativeVelocityX = fsvx - fpvx;
    relativeVelocityY = fsvy - fpvy;
    
    b = distX * relativeVelocityX + distY * relativeVelocityY;

    if (b < 0) { // possible collision
        dist = sqrt(distX * distX + distY * distY);
        relativeVelocitySq = relativeVelocityX * relativeVelocityX + relativeVelocityY * relativeVelocityY;
        d = (b * b) - (relativeVelocitySq) * (dist * dist - sumRadii * sumRadii);
        if (d > 0) {
            t = (- b - sqrt(d)) / relativeVelocitySq;

            if (t <= TIMESTEP) {
                CollisionEvent* ct = new CollisionEvent(primary, secondary, t, b, distX / dist, distY / dist);
                collisionPriority.push(ct);
                collisionEventMap[uniquePairingHash(primary, secondary)] = ct;
            }
        }   
    }

}

void calculateCollision2() {

    float sumMass;
    long unsigned i, j;
    double dt;

    for (i = 0; i < massL.size(); ++i) {
        for (j = i + 1; j < massL.size(); ++j) {
            doCollisionDetection(i, j);
        }

    }

    long unsigned int p, s, pk, sk;

    while (collisionPriority.size()) {
        //std::cout << "numCol: " << numberOfCollisions << std::endl;
        CollisionEvent* ct = collisionPriority.top();
        collisionPriority.pop();
        if (ct->isValid) {
            p = ct->primary;
            s = ct->secondary;
            sumMass = massL[s] + massL[p];
            
            positionXL[p] += velocityXL[p] * ct->time;
            positionYL[p] += velocityYL[p] * ct->time;
            positionXL[s] += velocityXL[s] * ct->time;
            positionYL[s] += velocityYL[s] * ct->time;
            // std::cout << "af p[" << p << "]: " << positionXL[p] << " | p[" << s << "]: " << positionXL[s] <<  " ct: "<< ct->time << std::endl;
 
            // std::cout << "b {} " << ct->b << std::endl;
           
            dt = TIMESTEP - ct->time;
          //  std::cout << "bf v[" << p << "]: " << velocityXL[p] << " | v[" << s << "]: " << velocityXL[s] <<  " ct: "<< ct->time << std::endl;
            velocityXL[p] += (   ((1.0f + eccentricity) * massL[s]) / sumMass) * ((ct->b * ct->uDx) / sumRadii);
            velocityYL[p] += (   ((1.0f + eccentricity) * massL[s]) / sumMass) * ((ct->b * ct->uDy) / sumRadii);
            velocityXL[s] += ( - ((1.0f + eccentricity) * massL[p]) / sumMass) * ((ct->b * ct->uDx) / sumRadii); 
            velocityYL[s] += ( - ((1.0f + eccentricity) * massL[p]) / sumMass) * ((ct->b * ct->uDy) / sumRadii);
            
            // std::cout << "at v[" << p << "]: " << velocityXL[p] << " | v[" << s << "]: " << velocityXL[s] <<  " ct: "<< ct->time << std::endl;
            
            // std::cout << "uDx[" << p << "]: " << ct->uDx << " | uDy[" << s << "]: " << ct->uDy <<  " ct: "<< ct->time << std::endl;
            
            positionXL[p] += velocityXL[p] * dt;
            positionYL[p] += velocityYL[p] * dt;
            positionXL[s] += velocityXL[s] * dt;
            positionYL[s] += velocityYL[s] * dt;

            // std::cout << "at v[" << p << "]: " << velocityXL[p] << " | v[" << s << "]: " << velocityXL[s] <<  " ct: "<< ct->time << std::endl;
            // dvx = ( ( massL[s]) / sumMass) * (ct->b * ct->uDx / sumRadii);
            // dvy = ( ( massL[s]) / sumMass) * (ct->b * ct->uDy / sumRadii);
            // positionXL[p] += dvx * dt;
            // positionYL[p] += dvy * dt;
            // std::cout << "before p[" << p << "]: " << positionXL[p] << " | p[" << s << "]: " << positionXL[s] <<  " ct: "<< ct->time << std::endl;

            // dvx = ( - ( massL[p]) / sumMass) * (ct->b * ct->uDx / sumRadii);
            // dvy = ( - ( massL[p]) / sumMass) * (ct->b * ct->uDy / sumRadii);
            // positionXL[s] += dvx * dt;
            // positionYL[s] += dvy * dt;

            
            collision[p] = 1;
            collision[s] = 1;
            
            collisionEventMap.erase(uniquePairingHash(p, s));

            for (i = 0; i < massL.size(); ++i) {
                if (p != i && s != i) {
                    pk = uniquePairingHash(i, p);
                    sk = uniquePairingHash(i, s);
                  //  std::cout <<"ps:" << uniquePairingHash(p, s) <<" | i:"<<i<<"| p:"<<p<<" | ip:"<<pk<<"| is:"<<sk<< " | numCol: " << numberOfCollisions << std::endl;
                        
                    if (collisionEventMap.find(pk) != collisionEventMap.end()) {
                        collisionEventMap[pk]->isValid = false;
                        collisionEventMap.erase(pk);
                        doCollisionDetection(i, p);
                    }
                    if (collisionEventMap.find(sk) != collisionEventMap.end()) {
                    //    std::cout << sk << " '' " << i << " numCol: " << numberOfCollisions << std::endl;
                        collisionEventMap[sk]->isValid = false;
                        collisionEventMap.erase(sk);
                        doCollisionDetection(i, s);
                    }
                } 
            }
            
        }
        delete ct;  
    }
    
}


void updateWorld() {


    calculateForce();
    calculateCollision2();
    calculatePosition();
    //calculateCollision();
    //std::cout << positionXL[0] << " af " << std::endl;
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
        
        pixels[(x + y * WIDTH) * 4 + 0] = massL[i] / 2;  // Red
        pixels[(x + y * WIDTH) * 4 + 1] = 255;  // Green
        pixels[(x + y * WIDTH) * 4 + 2] = 255;  // Blue
        pixels[(x + y * WIDTH) * 4 + 3] = 255;  // Alpha

        if (i == 0) {
            pixels[(x + y * WIDTH) * 4 + 0] = 255;  // Red
            pixels[(x + y * WIDTH) * 4 + 1] = 255;  // Green
            pixels[(x + y * WIDTH) * 4 + 2] = 255;  // Blue
            pixels[(x + y * WIDTH) * 4 + 3] = 255;  // Alpha
        }
    
    }
}