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

const int WIDTH = 800;
const int HEIGHT = 800;

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
void random500();
void planetAndSatellite();
void twoParticlesStraight();
void threeParticlesTriangle();
void planetAndMeteor();
void planetAndTidal();

void initWorld() {
    planetAndTidal();
}

void addParticle(int mass, float x, float y, float vx, float vy) {
    massL.push_back(mass);
    positionXL.push_back(x);
    positionYL.push_back(y);
    collision.push_back(0);
    velocityXL.push_back(vx);
    velocityYL.push_back(vy);
    accelXL.push_back(0);
    accelYL.push_back(0);
}

void random500() {
    long unsigned i, j;
    int x, y;
    float dist, minDist = 0.0f;
    float deltaX, deltaY;

    addParticle(100, 200, 200, 0, 0);
    for (i = 0; i < 500; i++) {
        minDist = 0.0f;
        while(minDist <= sumRadii) {
            x = (int) (std::rand() % 790) + 10;
            y = (int) (std::rand() % 790) + 10;
            for (j = 0; j < massL.size(); ++j) {
                deltaX = positionXL[j] - x;
                deltaY = positionYL[j] - y;
                dist = sqrt(deltaX * deltaX + deltaY * deltaY);
                if (dist < minDist || j == 0) minDist = dist;
            }
        }
        addParticle(100, x, y, 0, 0);
    }
}

void twoParticlesStraight() {
    addParticle(1000, 200, 200, 0, 0);
    addParticle(1000, 300, 200, 0, 0);
}

void threeParticlesTriangle() {
    addParticle(1000, 200, 200, 0, 0);
    addParticle(1000, 250, 286.6, 0, 0);
    addParticle(1000, 300, 200, 0, 0);
}



void planetAndSatellite() {
    long unsigned i, j;
    int x, y;
    float dist, minDist = 0.0f;
    float deltaX, deltaY;
    
    float distance = 300;

    float coreMassS = 100;
    float childMassS = 30;

    float coreMassP = 1000;
    float childMassP = 100;

    float corePosPX = 400;
    float corePosPY = 400;

    float corePosSX = corePosPX;
    float corePosSY = corePosPY - distance;

    float childCountS = 50;
    float childCountP = 100;


    float totalMassS = coreMassS + (childMassS * childCountS);
    float totalMassP = coreMassP + (childMassP * childCountP);
    
    int spaceBias = 50;
    int boxLengthP = sqrt(childCountP + spaceBias) * (sumRadii + 1);
    int boxLengthS = sqrt(childCountS + spaceBias) * (sumRadii + 1);

    double v = sqrt(G * (totalMassP) / distance);

    std::cout << "p bx:"<< boxLengthP << std::endl;
   
    std::cout << "px " << "min: " <<  corePosPX - (boxLengthP / 2) << " max: " << boxLengthP + corePosPX << std::endl;
    std::cout << "py " << "min: " <<  corePosPY - (boxLengthP / 2) << " max: " << boxLengthP + corePosPY << std::endl;

    addParticle(coreMassP, corePosPX, corePosPY, 0, 0);
    for (i = 0; i < childCountP; i++) {
        minDist = 0.0f;
        while(minDist <= sumRadii + 1) {
            x = (int) (std::rand() % boxLengthP) + corePosPX - (boxLengthP / 2);
            y = (int) (std::rand() % boxLengthP) + corePosPY - (boxLengthP / 2);
            for (j = 0; j < massL.size(); ++j) {
                deltaX = positionXL[j] - x;
                deltaY = positionYL[j] - y;
                dist = sqrt(deltaX * deltaX + deltaY * deltaY);
                if (dist < minDist || j == 0) minDist = dist;
            }
        }
        addParticle(childMassP, x, y, 0, 0);
    }
    //std::cout << v << std::endl;
    
    addParticle(coreMassS, corePosSX, corePosSY, v, 0);
    for (i = 0; i < childCountS; i++) {
        minDist = 0.0f;
        while(minDist <= sumRadii + 1) {
            x = (int) (std::rand() % boxLengthS) + corePosSX - (boxLengthS / 2);
            y = (int) (std::rand() % boxLengthS) + corePosSY - (boxLengthS / 2);
            for (j = 0; j < massL.size(); ++j) {
                deltaX = positionXL[j] - x;
                deltaY = positionYL[j] - y;
                dist = sqrt(deltaX * deltaX + deltaY * deltaY);
                if (dist < minDist || j == 0) minDist = dist;
            }
        }
        addParticle(childMassS, x, y, v, 0);
    }
}



void planetAndMeteor() {
    long unsigned i, j;
    int x, y;
    float dist, minDist = 0.0f;
    float deltaX, deltaY;
    
    float distance = 600;

    float coreMassS = 1000;
    float childMassS = 30;

    float coreMassP = 1000;
    float childMassP = 100;

    float corePosPX = 400;
    float corePosPY = 400;

    float corePosSX = corePosPX;
    float corePosSY = corePosPY - distance;

    float childCountS = 10;
    float childCountP = 100;


    float totalMassS = coreMassS + (childMassS * childCountS);
    float totalMassP = coreMassP + (childMassP * childCountP);
    
    int spaceBias = 50;
    int boxLengthP = sqrt(childCountP + spaceBias) * (sumRadii + 1);
    int boxLengthS = sqrt(childCountS + spaceBias) * (sumRadii + 1);


    std::cout << "p bx:"<< boxLengthP << std::endl;
   
    std::cout << "px " << "min: " <<  corePosPX - (boxLengthP / 2) << " max: " << boxLengthP + corePosPX << std::endl;
    std::cout << "py " << "min: " <<  corePosPY - (boxLengthP / 2) << " max: " << boxLengthP + corePosPY << std::endl;

    addParticle(coreMassP, corePosPX, corePosPY, 0, 0);
    for (i = 0; i < childCountP; i++) {
        minDist = 0.0f;
        while(minDist <= sumRadii + 1) {
            x = (int) (std::rand() % boxLengthP) + corePosPX - (boxLengthP / 2);
            y = (int) (std::rand() % boxLengthP) + corePosPY - (boxLengthP / 2);
            for (j = 0; j < massL.size(); ++j) {
                deltaX = positionXL[j] - x;
                deltaY = positionYL[j] - y;
                dist = sqrt(deltaX * deltaX + deltaY * deltaY);
                if (dist < minDist || j == 0) minDist = dist;
            }
        }
        addParticle(childMassP, x, y, 0, 0);
    }
    //std::cout << v << std::endl;
    
    double v = 300;
    addParticle(coreMassS, corePosSX, corePosSY, 0, v);
    for (i = 0; i < childCountS; i++) {
        minDist = 0.0f;
        while(minDist <= sumRadii + 1) {
            x = (int) (std::rand() % boxLengthS) + corePosSX - (boxLengthS / 2);
            y = (int) (std::rand() % boxLengthS) + corePosSY - (boxLengthS / 2);
            for (j = 0; j < massL.size(); ++j) {
                deltaX = positionXL[j] - x;
                deltaY = positionYL[j] - y;
                dist = sqrt(deltaX * deltaX + deltaY * deltaY);
                if (dist < minDist || j == 0) minDist = dist;
            }
        }
        addParticle(childMassS, x, y, 0, v);
    }
}


void planetAndTidal() {
    long unsigned i, j;
    int x, y;
    float dist, minDist = 0.0f;
    float deltaX, deltaY;
    
    float distance = 1000;

    float coreMassS = 1000000;
    float childMassS = 10000;

    float coreMassP = 1000;
    float childMassP = 100;

    float corePosPX = 300;
    float corePosPY = 500;

    float corePosSX = corePosPX + 200;
    float corePosSY = corePosPY - distance;

    float childCountS = 0;
    float childCountP = 100;


    float totalMassS = coreMassS + (childMassS * childCountS);
    float totalMassP = coreMassP + (childMassP * childCountP);
    
    int spaceBias = 50;
    int boxLengthP = sqrt(childCountP + spaceBias) * (sumRadii + 1);
    int boxLengthS = sqrt(childCountS + spaceBias) * (sumRadii + 1);


    std::cout << "p bx:"<< boxLengthP << std::endl;
   
    std::cout << "px " << "min: " <<  corePosPX - (boxLengthP / 2) << " max: " << boxLengthP + corePosPX << std::endl;
    std::cout << "py " << "min: " <<  corePosPY - (boxLengthP / 2) << " max: " << boxLengthP + corePosPY << std::endl;

    addParticle(coreMassP, corePosPX, corePosPY, 0, 0);
    for (i = 0; i < childCountP; i++) {
        minDist = 0.0f;
        while(minDist <= sumRadii + 1) {
            x = (int) (std::rand() % boxLengthP) + corePosPX - (boxLengthP / 2);
            y = (int) (std::rand() % boxLengthP) + corePosPY - (boxLengthP / 2);
            for (j = 0; j < massL.size(); ++j) {
                deltaX = positionXL[j] - x;
                deltaY = positionYL[j] - y;
                dist = sqrt(deltaX * deltaX + deltaY * deltaY);
                if (dist < minDist || j == 0) minDist = dist;
            }
        }
        addParticle(childMassP, x, y, 0, 0);
    }
    //std::cout << v << std::endl;
    
    double v = 200;
    addParticle(coreMassS, corePosSX, corePosSY, 0, v);
    for (i = 0; i < childCountS; i++) {
        minDist = 0.0f;
        while(minDist <= sumRadii + 1) {
            x = (int) (std::rand() % boxLengthS) + corePosSX - (boxLengthS / 2);
            y = (int) (std::rand() % boxLengthS) + corePosSY - (boxLengthS / 2);
            for (j = 0; j < massL.size(); ++j) {
                deltaX = positionXL[j] - x;
                deltaY = positionYL[j] - y;
                dist = sqrt(deltaX * deltaX + deltaY * deltaY);
                if (dist < minDist || j == 0) minDist = dist;
            }
        }
        addParticle(childMassS, x, y, 0, v);
    }
}


void calculateForce() {
    long unsigned i,j;
    float deltaX, deltaY;
    float distance;
    float distn;
    float directionX, directionY;
    float dAx, dAy;

    for (i = 0; i < massL.size(); ++i) {
        accelXL[i] = 0;
        accelYL[i] = 0;
    }

    for (i = 0; i < massL.size(); ++i) {
        for (j = i + 1; j < massL.size(); ++j) {
            deltaX = positionXL[j] - positionXL[i];
            deltaY = positionYL[j] - positionYL[i];
            distn = deltaX * deltaX + deltaY * deltaY;
            distance = sqrt(distn);
            directionX = deltaX / distance;
            directionY = deltaY / distance;

            dAx = (directionX * G) / distn;
            dAy = (directionY * G) / distn;

            accelXL[i] += dAx * massL[j];
            accelYL[i] += dAy * massL[j];
            accelXL[j] -= dAx * massL[i];
            accelYL[j] -= dAy * massL[i];   
        }
    }
}

void calculatePosition() {
    long unsigned i;
    for (i = 0; i < massL.size(); ++i) {
        if (!collision[i]) {
            velocityXL[i] += accelXL[i] * TIMESTEP;
            velocityYL[i] += accelYL[i] * TIMESTEP;
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
        double uDx, uDy;
        double rVx, rVy;
        double fpvx, fpvy, fsvx, fsvy;

        CollisionEvent(unsigned long primary, unsigned long secondary, double time, 
                    double fpvx, double fpvy, double fsvx, double fsvy,
                    double rVx, double rVy, double uDx, double uDy) {
            this->primary = primary;
            this->secondary = secondary;
            this->time = time;
            this->rVx = rVx;
            this->rVy = rVy;
            this->uDx = uDx;
            this->uDy = uDy;
            this->fpvx = fpvx;
            this->fpvy = fpvy;
            this->fsvx = fsvx;
            this->fsvy = fsvy;
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

    fpvx = velocityXL[primary]   + (accelXL[primary]   * TIMESTEP);
    fpvy = velocityYL[primary]   + (accelYL[primary]   * TIMESTEP); 
    fsvx = velocityXL[secondary] + (accelXL[secondary] * TIMESTEP);
    fsvy = velocityYL[secondary] + (accelYL[secondary] * TIMESTEP); 

    distX = positionXL[secondary] - positionXL[primary];
    distY = positionYL[secondary] - positionYL[primary];
    relativeVelocityX = fsvx - fpvx;
    relativeVelocityY = fsvy - fpvy;
    
    b = distX * relativeVelocityX + distY * relativeVelocityY;

    if (b < 0) { // possible collision
        dist = sqrt(distX * distX + distY * distY);
        // if (dist * dist - sumRadii * sumRadii <= 0) {
        //     CollisionEvent* ct = new CollisionEvent(primary, secondary, 0, fpvx, fpvy, fsvx, fsvy, relativeVelocityX, relativeVelocityY, distX / dist, distY / dist);
        //     collisionPriority.push(ct);
        //     collisionEventMap[uniquePairingHash(primary, secondary)] = ct;       
        // } else {
            relativeVelocitySq = relativeVelocityX * relativeVelocityX + relativeVelocityY * relativeVelocityY;
            d = (b * b) - (relativeVelocitySq) * (dist * dist - sumRadii * sumRadii);
            if (d > 0) {
                t = (- b - sqrt(d)) / relativeVelocitySq;
                if ( t <= TIMESTEP) {
                    CollisionEvent* ct = new CollisionEvent(primary, secondary, t, fpvx, fpvy, fsvx, fsvy, relativeVelocityX, relativeVelocityY, distX / dist, distY / dist);
                    collisionPriority.push(ct);
                    collisionEventMap[uniquePairingHash(primary, secondary)] = ct;
                }
            }   
        // }
    }

}

void calculateCollision2() {

    float sumMass;
    long unsigned i, j;
    double dt, b, m, distX, distY, dist, uDx, uDy;

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
            dt = TIMESTEP - ct->time;

            positionXL[p] += ct->fpvx * ct->time;
            positionYL[p] += ct->fpvy * ct->time;
            positionXL[s] += ct->fsvx * ct->time;
            positionYL[s] += ct->fsvy * ct->time;


            // std::cout << "b {} " << ct->b << std::endl;
            distX = positionXL[s] - positionXL[p];
            distY = positionYL[s] - positionYL[p];
            dist = sqrt(distX * distX + distY * distY);

            uDx = distX / dist;
            uDy = distY / dist;

            b = ct->rVx * distX + ct->rVy * distY;

            m = ((1.0f + eccentricity) / sumMass) * (b / sumRadii);
  
            ct->fpvx += m * massL[s] * uDx;
            ct->fpvy += m * massL[s] * uDy;
            ct->fsvx -= m * massL[p] * uDx; 
            ct->fsvy -= m * massL[p] * uDy;
            
            positionXL[p] += ct->fpvx * dt;
            positionYL[p] += ct->fpvy * dt;
            positionXL[s] += ct->fsvx * dt;
            positionYL[s] += ct->fsvy * dt;

            velocityXL[p] = ct->fpvx;
            velocityYL[p] = ct->fpvy;
            velocityXL[s] = ct->fsvx;
            velocityYL[s] = ct->fsvy;

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
        
        pixels[(x + y * WIDTH) * 4 + 0] = 100;  // Red
        pixels[(x + y * WIDTH) * 4 + 1] = 255;  // Green
        pixels[(x + y * WIDTH) * 4 + 2] = 255;  // Blue
        pixels[(x + y * WIDTH) * 4 + 3] = 255;  // Alpha
    
    }
}