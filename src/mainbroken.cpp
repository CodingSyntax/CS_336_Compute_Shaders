#include "config.h"
#include <math.h>
#include "util.h"
#include <cstring>
#include <chrono>
#include <thread>
#include <cmath>

#include "dataHandle.h"

#define WG_SIZE_X 8
#define WG_SIZE_Y 8

extern char _binary_src_shaders_fragment_frag_start[];
extern char _binary_src_shaders_vertex_vert_start[];
extern char _binary_src_shaders_fragTex_frag_start[];
extern char _binary_src_shaders_vertTex_vert_start[];
extern char _binary_src_shaders_gravity_comp_start[];
extern char _binary_src_shaders_gravityCollide_comp_start[];
extern char _binary_src_shaders_color1_comp_start[];
extern char _binary_src_shaders_color2_comp_start[];
extern char _binary_src_shaders_color3_comp_start[];
extern char _binary_src_shaders_color4_comp_start[];
// extern char _binary_src_shaders_raytrace_comp_start[];

char *fragSrc;
char *vertSrc;
char *fragTexSrc;
char *vertTexSrc;
char *gravitySrc;
char *gravityCSrc;
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
    convert(_binary_src_shaders_vertex_vert_start, vertSrc);
    convert(_binary_src_shaders_fragTex_frag_start, fragTexSrc);
    convert(_binary_src_shaders_vertTex_vert_start, vertTexSrc);
    convert(_binary_src_shaders_gravity_comp_start, gravitySrc);
    convert(_binary_src_shaders_gravityCollide_comp_start, gravityCSrc);
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
    free(fragTexSrc);
    free(vertTexSrc);
    free(gravitySrc);
    free(gravityCSrc);
    free(color1Src);
    free(color2Src);
    free(color3Src);
    free(color4Src);
}

std::vector<float> verts;


const int WIDTH = 400;
const int HEIGHT = 400;

int mode = 0;

float massSel = 10;

bool pause = true;

GLFWwindow* window;
unsigned int shaderProgram;
unsigned int computeShader;

unsigned int colorBuffer;

unsigned int forceBuffer, moreParticleBuffer, particleBuffer;


// VERTICES DATA
unsigned int verticesBuffer;
unsigned int texCoordsBuffer;

// TEXTURE DATA
GLuint textureID;
GLubyte* pixels;
GLint textureSamplerLocation;

// UNIVERSE DATA
Data3 particleData;
Data2 forceData;
Data5 moreParticleData;
std::vector<float> velocityXL;
std::vector<float> velocityYL;
std::vector<unsigned long int[2]> combination;

const float G = 10;
const float TIMESTEP = 0.02;

//std::chrono::milliseconds timespan ((int) (0.02 * 1000));

// UNIVERSE FUNCTIONS (USED IN MAIN, DRAW, AND UPDATE)

void initWorld(unsigned int particles);
void initWorldC(unsigned int particles);
void updateWorld();
void updateWorldC();
void addParticle(float mass, float x, float y, float vx, float vy);
void addParticleC(float mass, float x, float y, float vx, float vy);
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
    // std::cout << "compGrav";
    
    glUseProgram(computeShader);
    const int WGX = 64;
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particleData.getFullSize(), particleData.getData(), GL_STATIC_DRAW);
    std::cout << "ssb" << std::endl;
    
    forceData.zero();
    std::cout << "zero" << std::endl;
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, forceBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, forceData.getFullSize(), forceData.getData(), GL_DYNAMIC_READ);
    // std::cout << "b4 disp";
    //ceiling math
    glDispatchCompute((particleData.getSize() + WGX - 1) / WGX, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    // std::cout << "membar";
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, forceBuffer);
    float *p = (float*) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
    // std::cout << "mapbuff";
    float *s = p;
    for (; p < (s + forceData.getSize()); p++) {
        std::cout << *p << std::endl;
    }
    
    memcpy(forceData.getData(), p, forceData.getFullSize());
    // std::cout << "memcpy";
    
    glUseProgram(GL_NONE);
    // std::cout << "end grav";
    
}

void computeGravityC() {
    std::cout << "before:" << std::endl;
    for (int i = 0; i < moreParticleData.getSize(); i++) {
        std::cout << "(" << moreParticleData[i][0] << "," << moreParticleData[i][1] << "," << moreParticleData[i][2] << "," << moreParticleData[i][3] << "," << moreParticleData[i][4] << ")";
    }
    std::cout << std::endl;
    glUseProgram(computeShader);
    const int WGX = 64;
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, moreParticleBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, moreParticleData.getFullSize(), moreParticleData.getData(), GL_STATIC_DRAW);
    
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
    
    glUseProgram(GL_NONE);
    
    std::cout << "after:" << std::endl;
    for (int i = 0; i < moreParticleData.getSize(); i++) {
        std::cout << "(" << moreParticleData[i][0] << "," << moreParticleData[i][1] << "," << moreParticleData[i][2] << "," << moreParticleData[i][3] << "," << moreParticleData[i][4] << ")";
    }
    std::cout << std::endl;
}


void drawMassText() {
    glFrontFace(GL_CCW);
    std::ostringstream strStream;
    strStream << "Gravity Demo - Mass: " << massSel;
    glfwSetWindowTitle(window, strStream.str().c_str());
}

void drawGravity() {
    if (!pause) computeGravity();
    drawGravSim();
    drawMassText();
    
    glfwSwapBuffers(window);
    glfwPollEvents();
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
    while (!glfwWindowShouldClose(window)) {
        drawComputeShader();
    }
}

void color2() {
    std::cout << "color2" << std::endl;
    colorGeneral();
    
    computeShader = createShaderProgram(color2Src, (std::string)"color2");
    
    //draw
    while (!glfwWindowShouldClose(window)) {
        drawComputeShader();
    }
}

void color3() {
    std::cout << "color3" << std::endl;
    colorGeneral();
    
    computeShader = createShaderProgram(color3Src, (std::string)"color3");
    
    //draw
    while (!glfwWindowShouldClose(window)) {
        drawComputeShader();
    }
}

void color4() {
    std::cout << "color4" << std::endl;
    colorGeneral();
    
    computeShader = createShaderProgram(color4Src, (std::string)"color4");
    
    //draw
    while (!glfwWindowShouldClose(window)) {
        drawComputeShader();
    }
}

void gravity(unsigned int particles) {
    std::cout << "gravity" << std::endl;
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
    
    
    initWorld(particles);
    
    shaderProgram = createShaderProgram(vertSrc, fragSrc, "vert", "frag");
    computeShader = createShaderProgram(gravitySrc, (std::string)"gravity");
    // verticesBuffer = createAndLoadBuffer(particleData);
    verticesBuffer = createAndLoadBuffer(verts);
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glBufferData(GL_ARRAY_BUFFER, particleData.getFullSize(), particleData.getData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

  
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glPointSize(3);
    //draw
    
    
    while (!glfwWindowShouldClose(window)) {
        //std::cout << positionXL[0] << std::endl;
        drawGravity();
        if (!pause) updateWorld();
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
    verticesBuffer = createAndLoadBuffer(moreParticleData);
  
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glPointSize(3);
    //draw    
    while (!glfwWindowShouldClose(window)) {
        //std::cout << positionXL[0] << std::endl;
        drawGravityC();
        if (!pause) updateWorldC();
        //std::this_thread::sleep_for(timespan);
    }
}

int main(int argc, char *argv[]) {
    std::cout << "Starting... " << std::endl;
    convertBinaryToSrc();
    
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
    
    unsigned int particles = 10;
    if (argc > 1) {
        char *end;
        
        if (strcmp(argv[1], "color1") == 0) color1();
        else if (strcmp(argv[1], "color2") == 0) color2();
        else if (strcmp(argv[1], "color3") == 0) color3();
        else if (strcmp(argv[1], "color4") == 0) color4();
        else if (strcmp(argv[1], "gravity") == 0) {
            if (argc > 2) {
                particles = (int)strtol(argv[2], &end, 10);
                if (*end) particles = 10;
            }
            gravity(particles);
        } else if (strcmp(argv[1], "gravityC") == 0) {
            if (argc > 2) {
                particles = (int)strtol(argv[2], &end, 10);
                if (*end) particles = 10;
            }
            gravityC(particles);
        } else { //default
            particles = (int)strtol(argv[1], &end, 10);
            if (*end) particles = 10;
            gravity(particles);
        }
    } else gravity(particles);
    
    glfwDestroyWindow(window);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    
    freeSrc();
    
    std::cout << "Exit Success" << std::endl;
    exit(EXIT_SUCCESS);
}

// UNIVERSE FUNCTIONS IMPLEMENTATION
void initWorld(unsigned int particles) {
    long unsigned i, j;
    //int r1, r2;
    int x, y;
    float dist, minDist = 0.0f;
    float sumRadii = 2.0f;
    float deltaX, deltaY;
    addParticle(100, 200, 200, 0, 0);
    for (i = 0; i < particles; i++) {
    // r1 = std::rand() % 2 ? 1 : -1;
    // r2 = std::rand() % 2 ? 1 : -1;
        minDist = 0.0f;
        while(minDist < sumRadii) {
            x = (int) (std::rand() % 400);
            y = (int) (std::rand() % 400);
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
            x = (int) (std::rand() % 400);
            y = (int) (std::rand() % 400);
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
    float force[2] = {0,0};
    forceData.add(force);
    // accelXL.push_back(0);
    // accelYL.push_back(0);
}

void addParticleC(float mass, float x, float y, float vx, float vy) {
    float data[5] = {x, y, mass, 0, 0};
    moreParticleData.add(data);
    float force[2] = {0,0};
    forceData.add(force);
}

void updateWorld() {
    // CALCULATE VELOCITIES, POSITIONS
    long unsigned size = particleData.getSize();
    long unsigned i;
    for(i = 0 ; i < size; ++i) {
        velocityXL[i] += (forceData[i][0] / particleData[i][2]) * TIMESTEP;
        velocityYL[i] += (forceData[i][1] / particleData[i][2]) * TIMESTEP;
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