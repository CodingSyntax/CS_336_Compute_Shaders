#version 330 core

// in vec3 f_Color;
// in vec2 texCoords;
out vec4 fragmentColor;

// uniform sampler2D textureSampler;

void main() {
    fragmentColor = vec4(1, 1, 1, 1);//texture(textureSampler, texCoords);
}