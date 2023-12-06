#version 330 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec3 a_Color;

out vec3 fragmentColor;

void main() {
    gl_Position = vec4(a_Position, 0.0, 1);
    fragmentColor = a_Color;
}