#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;
out vec3 f_Color;
uniform vec2 scale;
uniform float zoom;
uniform vec2 transform;
// layout(location = 1) in vec2 a_TexCoords;

out vec2 texCoords;

void main() {
    float scaleX2 = (scale.x/2);
    float scaleY2 = (scale.y/2);
    float zoomFac = pow(1.01, zoom);
    // f_Color = vec3(max(0.2, abs(a_Color.x) / 10000), max(0.2, abs(a_Color.y) / 10000), max(0.2, abs(a_Color.z) / 100000));
    f_Color = vec3(max(0.2, log(abs(a_Color.x))/10), max(0.2, log(abs(a_Color.y))/10), max(0.2, log(abs(a_Position.z))/10));
    gl_Position = vec4((a_Position.x + (transform.x * zoomFac/100) - scaleX2) / zoomFac, (a_Position.y + (transform.y * zoomFac/100) - scaleY2) / zoomFac, 0.0, 1);
    // texCoords = a_TexCoords;
}
#end