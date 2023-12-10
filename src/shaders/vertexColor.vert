#version 330 core

layout(location = 0) in vec3 a_Position;
out vec3 f_Color;
uniform vec2 scale;

out vec2 texCoords;

void main() {
    float scaleX2 = (scale.x/2);
    float scaleY2 = (scale.y/2);
    if (a_Position.z < 0) {
        f_Color = vec3(1,0,0);
    } else f_Color = vec3(0,0,1);
    gl_Position = vec4((a_Position.x - scaleX2)/ scaleX2, (a_Position.y - scaleY2) / scaleY2, 0.0, 1);
}
#end