#version 330 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 texCoords;

void main() {
    gl_Position = vec4(a_Position, 0.0, 1);
    texCoords = a_TexCoords;
}
#end