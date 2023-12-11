#include "config.h"
#include <math.h>
#include "util.h"
#include <cstring>
#include <chrono>
#include <thread>
#include <cmath>
#include <bits/stdc++.h>
#include <algorithm>

#include "dataHandle.h"

#define MODE_GRAV 0
#define MODE_GRAV_C 1
#define MODE_GRAV_C2 2
#define MODE_COLOR 3
#define MODE_MAG 4

#define WG_SIZE_X 8
#define WG_SIZE_Y 8

extern char _binary_src_shaders_fragment_frag_start[];
extern char _binary_src_shaders_velColorF_frag_start[];
extern char _binary_src_shaders_fragmentColor_frag_start[];
extern char _binary_src_shaders_vertex_vert_start[];
extern char _binary_src_shaders_velColorV_vert_start[];
extern char _binary_src_shaders_vertexColor_vert_start[];
extern char _binary_src_shaders_fragTex_frag_start[];
extern char _binary_src_shaders_vertTex_vert_start[];
extern char _binary_src_shaders_gravity_comp_start[];
extern char _binary_src_shaders_magnetism_comp_start[];
extern char _binary_src_shaders_gravityC2_comp_start[];
extern char _binary_src_shaders_gravityCollide_comp_start[];
extern char _binary_src_shaders_collisions_comp_start[];
extern char _binary_src_shaders_color1_comp_start[];
extern char _binary_src_shaders_color2_comp_start[];
extern char _binary_src_shaders_color3_comp_start[];
extern char _binary_src_shaders_color4_comp_start[];
// extern char _binary_src_shaders_raytrace_comp_start[];

char *fragSrc;
char *vertSrc;
char *velColorVSrc;
char *velColorFSrc;
char *fragColorSrc;
char *vertColorSrc;
char *fragTexSrc;
char *vertTexSrc;
char *gravitySrc;
char *magnetismSrc;
char *gravityCSrc;
char *gravityC2Src;
char *collisionsSrc;
char *color1Src;
char *color2Src;
char *color3Src;
char *color4Src;

bool replace(std::string& str, const std::string& from) {
    size_t startPos = str.find(from);
    if(startPos == std::string::npos) return false;
    str[startPos] = '\0';
    str.assign(str.c_str());
    return true;
}

void convert(char binary[], char *&src) {
    std::string str = std::string(binary);
    replace(str, "#end");
    src = (char*)calloc(str.size() / (sizeof(char)), sizeof(char));
    strcpy(src, str.c_str());
}

void convertBinaryToSrc() {
    convert(_binary_src_shaders_fragment_frag_start, fragSrc);
    convert(_binary_src_shaders_velColorF_frag_start, velColorFSrc);
    convert(_binary_src_shaders_fragmentColor_frag_start, fragColorSrc);
    convert(_binary_src_shaders_vertex_vert_start, vertSrc);
    convert(_binary_src_shaders_velColorV_vert_start, velColorVSrc);
    convert(_binary_src_shaders_vertexColor_vert_start, vertColorSrc);
    convert(_binary_src_shaders_fragTex_frag_start, fragTexSrc);
    convert(_binary_src_shaders_vertTex_vert_start, vertTexSrc);
    convert(_binary_src_shaders_gravity_comp_start, gravitySrc);
    convert(_binary_src_shaders_magnetism_comp_start, magnetismSrc);
    convert(_binary_src_shaders_gravityCollide_comp_start, gravityCSrc);
    convert(_binary_src_shaders_gravityC2_comp_start, gravityC2Src);
    convert(_binary_src_shaders_collisions_comp_start, collisionsSrc);
    convert(_binary_src_shaders_color1_comp_start, color1Src);
    convert(_binary_src_shaders_color2_comp_start, color2Src);
    convert(_binary_src_shaders_color3_comp_start, color3Src);
    convert(_binary_src_shaders_color4_comp_start, color4Src);
    
    // std::cout << "Frag:\n" << fragSrc << std::endl;
    // std::cout << "Vert:\n" << vertSrc << std::endl;
    // std::cout << "Grav:\n" << gravitySrc << std::endl;
    // std::cout << "GravC:\n" << gravityCSrc << std::endl;
    // std::cout << "Color1:\n" << color1Src << std::endl;
    // std::cout << "Color2:\n" << color2Src << std::endl;
    // std::cout << "Color3:\n" << color3Src << std::endl;
    // std::cout << "Color4:\n" << color4Src << std::endl;
}

void freeSrc() {
    free(fragSrc);
    free(vertSrc);
    free(velColorFSrc);
    free(velColorVSrc);
    free(fragColorSrc);
    free(vertColorSrc);
    free(fragTexSrc);
    free(vertTexSrc);
    free(gravitySrc);
    free(magnetismSrc);
    free(gravityCSrc);
    free(gravityC2Src);
    free(collisionsSrc);
    free(color1Src);
    free(color2Src);
    free(color3Src);
    free(color4Src);
}


#define WIDTH 800
#define HEIGHT 800

float zoom = log(WIDTH / 2) / log(1.01);

float panX = 0;
float panY = 0;

int mode = 0;

double lastTime;
int nbFrames;

float massSel = 10;

bool pause = true;

GLFWwindow* window;
unsigned int shaderProgram;
unsigned int computeShader;
unsigned int computeShader2;

unsigned int colorBuffer;

unsigned int forceBuffer, moreParticleBuffer, particleBuffer;

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
Data2 forceData;
Data5 moreParticleData;
Data2 velocity;
std::vector<float> velocityXL;
std::vector<float> velocityYL;

std::vector<bool> collision;

const float G = 5;
const float TIMESTEP = 0.01;
const float sumRadii = 15.0f;
const float eccentricity = 0.5f;

//std::chrono::milliseconds timespan ((int) (0.02 * 1000));

// UNIVERSE FUNCTIONS (USED IN MAIN, DRAW, AND UPDATE)

void initWorld(unsigned int particles);
void initWorld(unsigned int particles, unsigned int mode);
void initWorldC(unsigned int particles);
void initWorldC2(unsigned int particles);
void updateWorld();
void updateWorldM();
void updateWorldC();
void updateWorldC2();
void initWorldM(unsigned int particles);
void addParticle(float mass, float x, float y, float vx, float vy);
void addParticleM(float mass, float x, float y, float vx, float vy);
void addParticleC(float mass, float x, float y, float vx, float vy);
void addParticleC2(float mass, float x, float y, float vx, float vy);
void renderParticle();
void calculateCollision2();

void resetCam() {
    zoom = log(WIDTH / 2) / log(1.01);
    panX = 0;
    panY = 0;
}

void pauseSim() {
    pause = !pause;
}

void createMass(GLFWwindow* window) {
    if (mode == MODE_COLOR) return;
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    double thisX = (x/width) * WIDTH;
    double thisY = HEIGHT - ((y/height) * HEIGHT);
    if (mode == MODE_GRAV) addParticle(massSel, thisX, thisY, 0, 0);
    if (mode == MODE_GRAV_C) addParticleC(massSel, thisX, thisY, 0, 0);
    if (mode == MODE_GRAV_C2) addParticleC2(massSel, thisX, thisY, 0, 0);
    if (mode == MODE_MAG) addParticleM(massSel, thisX, thisY, 0, 0);
    std::cout << "(" << thisX << "," << thisY << ")" << std::endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        pauseSim();
    if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        if (mods == GLFW_MOD_CONTROL) massSel += 10;
        else if (mods == GLFW_MOD_ALT) massSel += 100;
        else if (mods == (GLFW_MOD_CONTROL | GLFW_MOD_ALT)) massSel += 1000;
        else if (mods == GLFW_MOD_SHIFT) massSel += 0.1f;
        else if (mods == (GLFW_MOD_SHIFT | GLFW_MOD_CONTROL)) massSel += 0.01f;
        else if (mods == (GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT)) massSel += 0.001f;
        else massSel++;
    }
    if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        if (mods == GLFW_MOD_CONTROL) massSel -= 10;
        else if (mods == GLFW_MOD_ALT) massSel -= 100;
        else if (mods == (GLFW_MOD_CONTROL | GLFW_MOD_ALT)) massSel -= 1000;
        else if (mods == GLFW_MOD_SHIFT) massSel -= 0.1f;
        else if (mods == (GLFW_MOD_SHIFT | GLFW_MOD_CONTROL)) massSel -= 0.01f;
        else if (mods == (GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT)) massSel -= 0.001f;
        else massSel--;
    }
    if (key == GLFW_KEY_R && (action == GLFW_PRESS)) {
        resetCam();
    }
}

bool middlePressed = false;

void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
        createMass(window); //TODO: change to right click, and allow left click to pan the screen?
    if (button == GLFW_MOUSE_BUTTON_3 && action == GLFW_PRESS)
        middlePressed = true;
    if (button == GLFW_MOUSE_BUTTON_3 && action == GLFW_RELEASE)
        middlePressed = false;
}

double lastX = -1, lastY;

static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (middlePressed) {
        if (lastX == -1) {
            lastX = xpos;
            lastY = ypos;
        }
        panX += xpos - lastX;
        panY -= ypos - lastY;
        lastX = xpos;
        lastY = ypos;
    } else {
        lastX = -1;
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // massSel += yoffset; //zoom
    zoom -= yoffset * 10;
}

void initTimer() {
    lastTime = glfwGetTime();
    nbFrames = 0;
}

void tickTimer() {
    double currentTime = glfwGetTime();
    nbFrames++;
    if (currentTime - lastTime >= 1) {
        std::cout << (1000.0 / nbFrames) << "ms/frame" << std::endl;
        nbFrames = 0;
        lastTime = currentTime;
    }
}

void drawGravSim() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    // std::cout << "use" << std::endl;

    unsigned int pos = glGetAttribLocation(shaderProgram, "a_Position");
    unsigned int color = glGetAttribLocation(shaderProgram, "a_Color");
    // std::cout << pos << std::endl;

    glEnableVertexAttribArray(pos);
    glEnableVertexAttribArray(color);
    // std::cout << "attr" << std::endl;
    
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glVertexAttribPointer(pos, 3, GL_FLOAT, false, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glVertexAttribPointer(color, 2, GL_FLOAT, false, 0, 0);
    glUniform2f(glGetUniformLocation(shaderProgram, "scale"), WIDTH, HEIGHT);
    glUniform1f(glGetUniformLocation(shaderProgram, "zoom"), zoom);
    glUniform2f(glGetUniformLocation(shaderProgram, "transform"), panX, panY);
    // std::cout << "b4draw" << std::endl;
    
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    
    glDrawArrays(GL_POINTS, 0, particleData.getSize());
    // std::cout << "draw" << std::endl;
    
    glDisableVertexAttribArray(pos);
    glUseProgram(GL_NONE);
    // std::cout << "done" << std::endl;
}

void drawMagSim() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    // std::cout << "use" << std::endl;

    unsigned int pos = glGetAttribLocation(shaderProgram, "a_Position");
    // std::cout << pos << std::endl;

    glEnableVertexAttribArray(pos);
    // std::cout << "attr" << std::endl;
    
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glVertexAttribPointer(pos, 3, GL_FLOAT, false, 0, 0);
    glUniform2f(glGetUniformLocation(shaderProgram, "scale"), WIDTH, HEIGHT);
    glUniform1f(glGetUniformLocation(shaderProgram, "zoom"), zoom);
    glUniform2f(glGetUniformLocation(shaderProgram, "transform"), panX, panY);
    // std::cout << "b4draw" << std::endl;
    
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    
    glDrawArrays(GL_POINTS, 0, particleData.getSize());
    // std::cout << "draw" << std::endl;
    
    glDisableVertexAttribArray(pos);
    glUseProgram(GL_NONE);
    // std::cout << "done" << std::endl;
}

void drawGravCSim() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    
    unsigned int pos = glGetAttribLocation(shaderProgram, "a_Position");

    glEnableVertexAttribArray(pos);
    
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glVertexAttribPointer(pos, 2, GL_FLOAT, false, sizeof(float) * 5, 0);
    glUniform2f(glGetUniformLocation(shaderProgram, "scale"), WIDTH, HEIGHT);
    glUniform1f(glGetUniformLocation(shaderProgram, "zoom"), zoom);
    glUniform2f(glGetUniformLocation(shaderProgram, "transform"), panX, panY);
    
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    
    glDrawArrays(GL_POINTS, 0, moreParticleData.getSize());
    
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
    //ceiling (the amount of workgroups needed)
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
    // std::cout << "before:" << std::endl;
    // for (int i = 0; i < moreParticleData.getSize(); i++) {
    //     std::cout << "(" << moreParticleData[i][0] << "," << moreParticleData[i][1] << "," << moreParticleData[i][2] << "," << moreParticleData[i][3] << "," << moreParticleData[i][4] << ")";
    // }
    // std::cout << std::endl;
    glUseProgram(computeShader);
    const int WGX = 64;
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particleData.getFullSize(), particleData.getData(), GL_STATIC_DRAW);
    
    forceData.zero();
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, forceBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, forceData.getFullSize(), forceData.getData(), GL_DYNAMIC_READ);
    //ceiling math
    glDispatchCompute((particleData.getSize() + WGX - 1) / WGX, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, forceBuffer);
    void *p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
    
    memcpy(forceData.getData(), p, forceData.getFullSize());
    
    glUseProgram(GL_NONE);
    // std::cout << "after:" << std::endl;
    // for (int i = 0; i < moreParticleData.getSize(); i++) {
    //     std::cout << "(" << moreParticleData[i][0] << "," << moreParticleData[i][1] << "," << moreParticleData[i][2] << "," << moreParticleData[i][3] << "," << moreParticleData[i][4] << ")";
    // }
    // std::cout << std::endl;
}

void computeGravityC() {
    // std::cout << "before:" << std::endl;
    // for (int i = 0; i < moreParticleData.getSize(); i++) {
    //     std::cout << "(" << moreParticleData[i][0] << "," << moreParticleData[i][1] << "," << moreParticleData[i][2] << "," << moreParticleData[i][3] << "," << moreParticleData[i][4] << ")";
    // }
    // std::cout << std::endl;
    glUseProgram(computeShader);
    const int WGX = 64;
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, moreParticleBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, moreParticleData.getFullSize(), moreParticleData.getData(), GL_DYNAMIC_READ);
    
    forceData.zero();
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, forceBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, forceData.getFullSize(), forceData.getData(), GL_DYNAMIC_READ);
    //ceiling math
    glDispatchCompute((moreParticleData.getSize() + WGX - 1) / WGX, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    //Copy force data back to cpu
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, forceBuffer);
    void *p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
    
    memcpy(forceData.getData(), p, forceData.getFullSize());
    
    //Copy particle data back to cpu
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, moreParticleBuffer);
    p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
    
    memcpy(moreParticleData.getData(), p, moreParticleData.getFullSize());
    
    //// COLLISIONS
    glUseProgram(computeShader2);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, moreParticleBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, moreParticleData.getFullSize(), moreParticleData.getData(), GL_DYNAMIC_READ);
    //ceiling math
    glDispatchCompute((moreParticleData.getSize() + WGX - 1) / WGX, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    //Copy particle data back to cpu
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, moreParticleBuffer);
    p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
    
    memcpy(moreParticleData.getData(), p, moreParticleData.getFullSize());
    
    glUseProgram(GL_NONE);
    
    // std::cout << "after:" << std::endl;
    // for (int i = 0; i < moreParticleData.getSize(); i++) {
    //     std::cout << "(" << moreParticleData[i][0] << "," << moreParticleData[i][1] << "," << moreParticleData[i][2] << "," << moreParticleData[i][3] << "," << moreParticleData[i][4] << ")";
    // }
    // std::cout << std::endl;
}

void drawMassText() {
    std::ostringstream strStream;
    strStream << "Gravity Demo - Mass: " << massSel;
    glfwSetWindowTitle(window, strStream.str().c_str());
}

void drawGravity() {
    if (!pause) computeGravity();
    drawGravSim();
    // std::cout << "text" << std::endl;
    drawMassText();
    // std::cout << "swap" << std::endl;
    
    glfwSwapBuffers(window);
    // std::cout << "poll" << std::endl;
    glfwPollEvents();
    // std::cout << "done" << std::endl;
}

void drawMag() {
    if (!pause) computeGravity();
    drawMagSim();
    // std::cout << "text" << std::endl;
    drawMassText();
    // std::cout << "swap" << std::endl;
    
    glfwSwapBuffers(window);
    // std::cout << "poll" << std::endl;
    glfwPollEvents();
    // std::cout << "done" << std::endl;
}

void drawGravityC() {
    if (!pause) computeGravityC();
    drawGravCSim();
    drawMassText();
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void colorGeneral() {
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
    
    
    shaderProgram = createShaderProgram(vertTexSrc, fragTexSrc, "vert", "frag");
    verticesBuffer = createAndLoadBuffer(verts);
    texCoordsBuffer = createAndLoadBuffer(texCoords);
    
    glClearColor(0.f, 0.f, 0.f, 1.0f);
}

void color1() {
    std::cout << "color1" << std::endl;
    colorGeneral();
    
    computeShader = createShaderProgram(color1Src, (std::string)"color1");
    
    //draw
    initTimer();
    while (!glfwWindowShouldClose(window)) {
        drawComputeShader();
        tickTimer();
    }
}

void color2() {
    std::cout << "color2" << std::endl;
    colorGeneral();
    
    computeShader = createShaderProgram(color2Src, (std::string)"color2");
    
    //draw
    initTimer();
    while (!glfwWindowShouldClose(window)) {
        drawComputeShader();
        tickTimer();
    }
}

void color3() {
    std::cout << "color3" << std::endl;
    colorGeneral();
    
    computeShader = createShaderProgram(color3Src, (std::string)"color3");
    initTimer();
    
    //draw
    initTimer();
    while (!glfwWindowShouldClose(window)) {
        drawComputeShader();
        tickTimer();
    }
}

void color4() {
    std::cout << "color4" << std::endl;
    colorGeneral();
    
    computeShader = createShaderProgram(color4Src, (std::string)"color4");
    
    //draw
    initTimer();
    while (!glfwWindowShouldClose(window)) {
        drawComputeShader();
    }
}

void gravity(unsigned int particles, unsigned int gravMode) {
    std::cout << "gravity" << std::endl;
    // std::cout << _binary_src_shaders_fragment_frag_size << std::endl;
    // char *fragStart = _binary_src_shaders_fragment_frag_start;
    // char *vertSrc = _binary_src_shaders_vertex_vert_start;
    // char *computeSrc = _binary_src_shaders_gravity_comp_start;
    
    // std::cout << "Frag:\n" << fragSrc << std::endl;
    
    glGenBuffers(1, &forceBuffer);
    glGenBuffers(1, &particleBuffer);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particleData.getFullSize(), particleData.getData(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particleBuffer);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, forceBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, forceData.getFullSize(), forceData.getData(), GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, forceBuffer);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    
    initWorld(particles, gravMode);
    
    shaderProgram = createShaderProgram(velColorVSrc, velColorFSrc, "velColorV", "velColorF");
    computeShader = createShaderProgram(gravitySrc, (std::string)"gravity");
    verticesBuffer = createAndLoadBuffer(verts);
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glBufferData(GL_ARRAY_BUFFER, particleData.getFullSize(), particleData.getData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    colorBuffer = createAndLoadBuffer(verts);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, velocity.getFullSize(), velocity.getData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

  
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glPointSize(3);
    //draw
    
    initTimer();
    while (!glfwWindowShouldClose(window)) {
        //std::cout << positionXL[0] << std::endl;
        drawGravity();
        if (!pause) updateWorld();
        tickTimer();
        //std::this_thread::sleep_for(timespan);
    }
}

void magnetism(unsigned int particles) {
    std::cout << "magnet" << std::endl;
    // std::cout << _binary_src_shaders_fragment_frag_size << std::endl;
    // char *fragStart = _binary_src_shaders_fragment_frag_start;
    // char *vertSrc = _binary_src_shaders_vertex_vert_start;
    // char *computeSrc = _binary_src_shaders_gravity_comp_start;
    
    // std::cout << "Frag:\n" << fragSrc << std::endl;
    
    glGenBuffers(1, &forceBuffer);
    glGenBuffers(1, &particleBuffer);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particleData.getFullSize(), particleData.getData(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particleBuffer);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, forceBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, forceData.getFullSize(), forceData.getData(), GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, forceBuffer);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    
    initWorldM(particles);
    
    shaderProgram = createShaderProgram(vertColorSrc, fragColorSrc, "vert", "frag");
    computeShader = createShaderProgram(magnetismSrc, (std::string)"magnet");
    verticesBuffer = createAndLoadBuffer(verts);
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glBufferData(GL_ARRAY_BUFFER, particleData.getFullSize(), particleData.getData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

  
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glPointSize(3);
    //draw
    
    initTimer();
    while (!glfwWindowShouldClose(window)) {
        //std::cout << positionXL[0] << std::endl;
        drawMag();
        if (!pause) updateWorldM();
        tickTimer();
        //std::this_thread::sleep_for(timespan);
    }
}

void gravityC(unsigned int particles) {
    std::cout << "gravityC" << std::endl;
    // std::cout << "Frag:\n" << fragSrc << std::endl;
    
    glGenBuffers(1, &moreParticleBuffer);
    glGenBuffers(1, &forceBuffer);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, moreParticleBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, moreParticleData.getFullSize(), moreParticleData.getData(), GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, moreParticleBuffer);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, forceBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, forceData.getFullSize(), forceData.getData(), GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, forceBuffer);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    
    initWorldC(particles);
    
    shaderProgram = createShaderProgram(vertSrc, fragSrc, "vert", "frag");
    computeShader = createShaderProgram(gravityCSrc, (std::string)"gravityC");
    computeShader2 = createShaderProgram(collisionsSrc, (std::string)"collisions");
    verticesBuffer = createAndLoadBuffer(verts);
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glBufferData(GL_ARRAY_BUFFER, moreParticleData.getFullSize(), moreParticleData.getData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glPointSize(3);
    //draw    
    initTimer();
    while (!glfwWindowShouldClose(window)) {
        //std::cout << positionXL[0] << std::endl;
        drawGravityC();
        if (!pause) updateWorldC();
        tickTimer();
        //std::this_thread::sleep_for(timespan);
    }
}

void gravityC2(unsigned int particles) {
    std::cout << "gravityC2" << std::endl;
    
    glGenBuffers(1, &forceBuffer);
    glGenBuffers(1, &particleBuffer);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particleData.getFullSize(), particleData.getData(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particleBuffer);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, forceBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, forceData.getFullSize(), forceData.getData(), GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, forceBuffer);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    initWorldC2(particles);
    
    shaderProgram = createShaderProgram(vertSrc, fragSrc, "vert", "frag");
    computeShader = createShaderProgram(gravityC2Src, (std::string)"gravityC2");
    verticesBuffer = createAndLoadBuffer(verts);
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glBufferData(GL_ARRAY_BUFFER, particleData.getFullSize(), particleData.getData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glPointSize(sumRadii / 2);
    
    //draw    
    initTimer();
    while (!glfwWindowShouldClose(window)) {
        //std::cout << positionXL[0] << std::endl;
        // std::cout << "drawgravityC2" << std::endl;
        
        drawGravity();
        if (!pause) updateWorldC2();
        tickTimer();
        //std::this_thread::sleep_for(timespan);
    }
}

int main(int argc, char *argv[]) {
    std::cout << "Starting... " << std::endl;
    convertBinaryToSrc();
    
    // char *color2Src = _binary_src_shaders_color2_comp_start;
    // char *color3Src = _binary_src_shaders_color3_comp_start;
    // char *raytraceSrc = _binary_src_shaders_raytrace_comp_start;
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
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    
    unsigned int particles = 10;
    unsigned int gravMode = 0;
    mode = MODE_GRAV_C2;
    if (argc > 1) {
        char *end;
        
        if (strcmp(argv[1], "color1") == 0) {
            mode = MODE_COLOR;
            color1();
        } else if (strcmp(argv[1], "color2") == 0) {
            mode = MODE_COLOR;
            color2();
        } else if (strcmp(argv[1], "color3") == 0) {
            mode = MODE_COLOR;
            color3();
        } else if (strcmp(argv[1], "color4") == 0) { 
            mode = MODE_COLOR;
            color4();
        } else if (strcmp(argv[1], "gravity") == 0) {
            mode = MODE_GRAV;
            if (argc > 2) {
                particles = (int)strtol(argv[2], &end, 10);
                if (*end) particles = 10;
                if (argc > 3) {
                    gravMode = (int)strtol(argv[3], &end, 10);
                    if (*end) gravMode = 0;
                }
            }
            gravity(particles, gravMode);
        } else if (strcmp(argv[1], "gravityC") == 0) {
            mode = MODE_GRAV_C;
            if (argc > 2) {
                particles = (int)strtol(argv[2], &end, 10);
                if (*end) particles = 10;
            }
            gravityC(particles);
        } else if (strcmp(argv[1], "gravityC2") == 0) {
            mode = MODE_GRAV_C2;
            if (argc > 2) {
                particles = (int)strtol(argv[2], &end, 10);
                if (*end) particles = 10;
            }
            gravityC2(particles);
        } else if (strcmp(argv[1], "magnet") == 0) {
            mode = MODE_MAG;
            if (argc > 2) {
                particles = (int)strtol(argv[2], &end, 10);
                if (*end) particles = 10;
            }
            magnetism(particles);
        } else { //default
            particles = (int)strtol(argv[1], &end, 10);
            if (*end) particles = 10;
            gravityC2(particles);
        }
    } else gravityC2(particles);
    
    glfwDestroyWindow(window);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    
    freeSrc();
    
    std::cout << "Exit Success" << std::endl;
    exit(EXIT_SUCCESS);
}
// UNIVERSE FUNCTIONS IMPLEMENTATION
void initWorld(unsigned int particles, unsigned int mode) {
    long unsigned i;
    //int r1, r2;
    int x, y;
    // float dist, minDist = 0.0f;
    // float sumRadii = 2.0f;
    // float deltaX, deltaY;
    
    if (mode == 3) {
        // float radius = 200;
        float mass = 2000000;
        addParticle(mass, WIDTH / 2, HEIGHT / 2, 0, 0);
        for (i = 0; i < particles; i++) {
        // r1 = std::rand() % 2 ? 1 : -1;
        // r2 = std::rand() % 2 ? 1 : -1;
            // minDist = 0.0f;
            // regen:
                x = (int) (std::rand() % (WIDTH / 2) + (WIDTH / 4));
                y = (int) (std::rand() % (HEIGHT / 2) + (HEIGHT / 4));
            //   std::cout << minDist << std::endl;
            float dX = x - (WIDTH / 2);
            float dY = (y - (HEIGHT / 2));
            float dist = sqrt(dX * dX + dY * dY);
            // if (dist > radius) goto regen;
            
            float mass2 = 2000 / (dist);
            float v = sqrt((G * (mass + mass2))/dist);
            addParticle(mass2, x, y, -v * dY/ dist, v * dX/ dist);
            // addParticle(100, x, y, y-200, x-200);
        }
    }
    
    if (mode == 2) {
        float mass = 2000000;
        addParticle(mass, WIDTH / 2, HEIGHT / 2, 0, 0);
        for (i = 0; i < particles; i++) {
        // r1 = std::rand() % 2 ? 1 : -1;
        // r2 = std::rand() % 2 ? 1 : -1;
            // minDist = 0.0f;
                x = (int) (std::rand() % (WIDTH / 2) + (WIDTH / 4));
                y = (int) (std::rand() % (HEIGHT / 2) + (HEIGHT / 4));
            //   std::cout << minDist << std::endl;
            
            float dX = x - (WIDTH / 2);
            float dY = y - (HEIGHT / 2);
            float dist = sqrt(dX * dX + dY * dY);
            float mass2 = (std::rand() % 2000 + 200);
            float mu = G * (mass + mass2);
            float a = 2 * mass2;
            float v = sqrt(mu * ((2/dist)-(1/a)));
            addParticle(mass2, x, y, -v * dY/ dist, v * dX/ dist);
            // addParticle(100, x, y, y-200, x-200);
        }
    }
    
    if (mode == 1) {
        addParticle(200, WIDTH / 2, HEIGHT / 2, 0, 0);
        for (i = 0; i < particles; i++) {
        // r1 = std::rand() % 2 ? 1 : -1;
        // r2 = std::rand() % 2 ? 1 : -1;
            // minDist = 0.0f;
                x = (int) (std::rand() % (WIDTH / 2) + (WIDTH / 4));
                y = (int) (std::rand() % (HEIGHT / 2) + (HEIGHT / 4));
            //   std::cout << minDist << std::endl;
            
            addParticle(100, x, y, y-(WIDTH / 2), x-(HEIGHT / 2));
        }
    }
    
    if (mode == 0) {
        addParticle(200, WIDTH / 2, HEIGHT / 2, 0, 0);
        for (i = 0; i < particles; i++) {
        // r1 = std::rand() % 2 ? 1 : -1;
        // r2 = std::rand() % 2 ? 1 : -1;
            // minDist = 0.0f;
                x = (int) (std::rand() % (WIDTH / 2) + (WIDTH / 4));
                y = (int) (std::rand() % (HEIGHT / 2) + (HEIGHT / 4));
            //   std::cout << minDist << std::endl;
            
            addParticle(100, x, y, 0, 0);
        }
    }
        
    //v = dqrt g*M/R
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

void initWorld(unsigned int particles) {
    initWorld(particles, 1);
}

void initWorldC(unsigned int particles) {
    long unsigned i, j;
    //int r1, r2;
    int x, y;
    float dist, minDist = 0.0f;
    float sumRadii = 2.0f;
    float deltaX, deltaY;
    addParticleC(100, 200, 200, 0, 0);
    for (i = 0; i < particles; i++) {
    // r1 = std::rand() % 2 ? 1 : -1;
    // r2 = std::rand() % 2 ? 1 : -1;
        minDist = 0.0f;
        while(minDist < sumRadii) {
            x = (int) (std::rand() % WIDTH);
            y = (int) (std::rand() % HEIGHT);
            for (j = 0; j < moreParticleData.getSize(); ++j) {
                deltaX = moreParticleData[j][0] - x;
                deltaY = moreParticleData[j][1] - y;
                dist = sqrt(deltaX * deltaX + deltaY * deltaY);
                if (dist < minDist || j == 0) minDist = dist;
            }
        //   std::cout << minDist << std::endl;
        }
        addParticleC(100, x, y, 0, 0);
    }
}

void initWorldM(unsigned int particles) {
    long unsigned i;
    //int r1, r2;
    int x, y;
    // float dist, minDist = 0.0f;
    // float sumRadii = 2.0f;
    // float deltaX, deltaY;
    
    addParticleM(200, WIDTH / 2, HEIGHT / 2, 0, 0);
    for (i = 0; i < particles; i++) {
    // r1 = std::rand() % 2 ? 1 : -1;
    // r2 = std::rand() % 2 ? 1 : -1;
        // minDist = 0.0f;
        x = (int) (std::rand() % (WIDTH / 2) + (WIDTH / 4));
        y = (int) (std::rand() % (HEIGHT / 2) + (HEIGHT / 4));
        //   std::cout << minDist << std::endl;
        // float mass = (std::rand() % 800 - 400);
        // std::cout << mass << std::endl;
        addParticleM(x < 400 ? -100 : 100, x, y, 0, 0);
    }
}

void initWorldC2(unsigned int particles) {
    long unsigned i, j;
    //int r1, r2;
    int x, y;
    float dist, minDist = 0.0f;
    float sumRadii = 2.0f;
    float deltaX, deltaY;
    addParticleC2(100, 200, 200, 0, 0);
    for (i = 0; i < particles; i++) {
    // r1 = std::rand() % 2 ? 1 : -1;
    // r2 = std::rand() % 2 ? 1 : -1;
        minDist = 0.0f;
        while(minDist < sumRadii) {
            x = (int) (std::rand() % WIDTH);
            y = (int) (std::rand() % HEIGHT);
            for (j = 0; j < particleData.getSize(); ++j) {
                deltaX = particleData[j][0] - x;
                deltaY = particleData[j][1] - y;
                dist = sqrt(deltaX * deltaX + deltaY * deltaY);
                if (dist < minDist || j == 0) minDist = dist;
            }
        //   std::cout << minDist << std::endl;
        }
        addParticleC2(100, x, y, 0, 0);
    }
}

void addParticle(float mass, float x, float y, float vx, float vy) {
    float data[3] = {x, y, mass};
    particleData.add(data);
    verts.push_back(x/WIDTH);
    verts.push_back(y/HEIGHT);
    
    // massL.push_back(mass);
    // positionXL.push_back(x);
    // positionYL.push_back(y);
    float v[2] = {vx, vy};
    velocity.add(v);
    // velocityXL.push_back(vx);
    // velocityYL.push_back(vy);
    float force[2] = {0,0};
    forceData.add(force);
    // accelXL.push_back(0);
    // accelYL.push_back(0);
}

void addParticleM(float mass, float x, float y, float vx, float vy) {
    float data[3] = {x, y, mass};
    particleData.add(data);
    verts.push_back(x/WIDTH);
    verts.push_back(y/HEIGHT);
    
    // massL.push_back(mass);
    // positionXL.push_back(x);
    // positionYL.push_back(y);
    velocityXL.push_back(vx);
    velocityYL.push_back(vy);
    float force[2] = {0,0};
    forceData.add(force);
    // accelXL.push_back(0);
    // accelYL.push_back(0);
}

void addParticleC(float mass, float x, float y, float vx, float vy) {
    float data[5] = {x, y, mass, vx, vy};
    moreParticleData.add(data);
    verts.push_back(x/WIDTH);
    verts.push_back(y/HEIGHT);
    float force[2] = {0,0};
    forceData.add(force);
}

void addParticleC2(float mass, float x, float y, float vx, float vy) {
    float data[3] = {x, y, mass};
    particleData.add(data);
    verts.push_back(x/WIDTH);
    verts.push_back(y/HEIGHT);
    velocityXL.push_back(vx);
    velocityYL.push_back(vy);
    float force[2] = {0,0}; //force is accel
    forceData.add(force);
    collision.push_back(0);
}

void calculatePositionC2() {
    // CALCULATE VELOCITIES, POSITIONS
    long unsigned size = particleData.getSize();
    long unsigned i;
    for(i = 0 ; i < size; ++i) {
        if (!collision[i]) { //force is accel
            velocityXL[i] += (forceData[i][0]) * TIMESTEP;
            velocityYL[i] += (forceData[i][1]) * TIMESTEP;
            particleData[i][0] += velocityXL[i] * TIMESTEP;
            particleData[i][1] += velocityYL[i] * TIMESTEP;
        }
        collision[i] = 0;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glBufferData(GL_ARRAY_BUFFER, particleData.getFullSize(), particleData.getData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void updateWorld() {
    long unsigned size = particleData.getSize();
    long unsigned i;
    for(i = 0 ; i < size; ++i) {
        velocity[i][0] += (forceData[i][0] / particleData[i][2]) * TIMESTEP;
        velocity[i][1] += (forceData[i][1] / particleData[i][2]) * TIMESTEP;
        particleData[i][0] += velocity[i][0] * TIMESTEP;
        particleData[i][1] += velocity[i][1] * TIMESTEP;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glBufferData(GL_ARRAY_BUFFER, particleData.getFullSize(), particleData.getData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, velocity.getFullSize(), velocity.getData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // for (int i = 0; i < particleData.getSize(); i++) {
    //     std::cout << "(" << particleData[i][0] << "," << particleData[i][1] << ")";
    // }
    // std::cout << std::endl;
}

void updateWorldM() {
    long unsigned size = particleData.getSize();
    long unsigned i;
    for(i = 0 ; i < size; ++i) {
        velocityXL[i] += (forceData[i][0] / abs(particleData[i][2])) * TIMESTEP;
        velocityYL[i] += (forceData[i][1] / abs(particleData[i][2])) * TIMESTEP;
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

void updateWorldC() {
    // CALCULATE VELOCITIES, POSITIONS
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glBufferData(GL_ARRAY_BUFFER, moreParticleData.getFullSize(), moreParticleData.getData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // for (int i = 0; i < moreParticleData.getSize(); i++) {
    //     std::cout << "(" << moreParticleData[i][0] << "," << moreParticleData[i][1] << ")";
    // }
    // std::cout << std::endl;
}

void updateWorldC2() {
    // CALCULATE VELOCITIES, POSITIONS
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glBufferData(GL_ARRAY_BUFFER, moreParticleData.getFullSize(), moreParticleData.getData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // for (int i = 0; i < moreParticleData.getSize(); i++) {
    //     std::cout << "(" << moreParticleData[i][0] << "," << moreParticleData[i][1] << ")";
    // }
    // std::cout << std::endl;
    calculateCollision2();
    calculatePositionC2();
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
    
    //forceData is accel
    fpvx = velocityXL[primary] + (forceData[primary][0] * TIMESTEP);
    fpvy = velocityYL[primary] + (forceData[primary][1] * TIMESTEP); 
    fsvx = velocityXL[secondary] + (forceData[secondary][0] * TIMESTEP);
    fsvy = velocityYL[secondary] + (forceData[secondary][1] * TIMESTEP); 

    distX = particleData[secondary][0] - particleData[primary][0];
    distY = particleData[secondary][1] - particleData[primary][1];
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
                CollisionEvent* ct = new CollisionEvent(primary, secondary, t, fpvx, fpvy, fsvx, fsvy, relativeVelocityX, relativeVelocityY, distX / dist, distY / dist);
                collisionPriority.push(ct);
                collisionEventMap[uniquePairingHash(primary, secondary)] = ct;
            }
        }   
    }

}

void calculateCollision2() {

    float sumMass;
    long unsigned i, j;
    double dt, b, m, distX, distY, dist, uDx, uDy;

    for (i = 0; i < particleData.getSize(); ++i) {
        for (j = i + 1; j < particleData.getSize(); ++j) {
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
            sumMass = particleData[s][2] + particleData[p][2];
            
            particleData[p][0] += ct->fpvx * ct->time;
            particleData[p][1] += ct->fpvy * ct->time;
            particleData[s][0] += ct->fsvx * ct->time;
            particleData[s][1] += ct->fsvy * ct->time;


            // std::cout << "b {} " << ct->b << std::endl;
            distX = particleData[s][0] - particleData[p][0];
            distY = particleData[s][1] - particleData[p][1];
            dist = sqrt(distX * distX + distY * distY);
            // if (dist - sumRadii > 0.001) {
            //     std::cout << "af p[" << p << "]: " << positionXL[p] << " | p[" << s << "]: " << positionXL[s] <<  " ct: "<< ct->time << std::endl;
            // } 

            uDx = distX / dist;
            uDy = distY / dist;

            b = ct->rVx * distX + ct->rVy * distY;

            dt = TIMESTEP - ct->time;
            m = ((1.0f + eccentricity) / sumMass) * (b / sumRadii);
          //  std::cout << "bf v[" << p << "]: " << velocityXL[p] << " | v[" << s << "]: " << velocityXL[s] <<  " ct: "<< ct->time << std::endl;
            ct->fpvx += m * particleData[s][2] * uDx;
            ct->fpvy += m * particleData[s][2] * uDy;
            ct->fsvx -= m * particleData[p][2] * uDx; 
            ct->fsvy -= m * particleData[p][2] * uDy;
            
            // std::cout << "at v[" << p << "]: " << velocityXL[p] << " | v[" << s << "]: " << velocityXL[s] <<  " ct: "<< ct->time << std::endl;
            
            // std::cout << "uDx[" << p << "]: " << ct->uDx << " | uDy[" << s << "]: " << ct->uDy <<  " ct: "<< ct->time << std::endl;
            
            particleData[p][0] += ct->fpvx * dt;
            particleData[p][1] += ct->fpvy * dt;
            particleData[s][0] += ct->fsvx * dt;
            particleData[s][1] += ct->fsvy * dt;

            velocityXL[p] = ct->fpvx;
            velocityYL[p] = ct->fpvy;
            velocityXL[s] = ct->fsvx;
            velocityYL[s] = ct->fsvy;

            collision[p] = 1;
            collision[s] = 1;
            
            collisionEventMap.erase(uniquePairingHash(p, s));

            for (i = 0; i < particleData.getSize(); ++i) {
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