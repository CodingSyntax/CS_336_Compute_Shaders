#version 330 core

layout(location = 0) in vec2 a_Position;
// layout(location = 1) in vec3 a_Color;
// out vec3 f_Color;
uniform vec2 scale;
// layout(location = 1) in vec2 a_TexCoords;

out vec2 texCoords;

void main() {
    float scaleX2 = (scale.x/2);
    float scaleY2 = (scale.y/2);
    // f_Color = a_Color;
    gl_Position = vec4((a_Position.x - scaleX2)/ scaleX2, (a_Position.y - scaleY2) / scaleY2, 0.0, 1);
    // texCoords = a_TexCoords;
}