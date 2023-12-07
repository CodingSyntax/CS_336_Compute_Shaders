#version 330 core

in vec2 texCoords;
out vec4 fragmentColor;

uniform sampler2D textureSampler;

void main() {
    fragmentColor = texture(textureSampler, texCoords);
}