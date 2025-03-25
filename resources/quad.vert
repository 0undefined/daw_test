#version 330 core

layout(location = 0) in vec3 pos;

out vec3 fragColor;

uniform mat4 MVP;

void main() {

    gl_Position =
    MVP * vec4( pos.x, 1, pos.y, 1);
    //vec4(pos.x, 1.0, pos.y, 1.0);

    fragColor.x = gl_Position.x;
    fragColor.y = gl_Position.y;
    fragColor.z = gl_Position.z;
}
