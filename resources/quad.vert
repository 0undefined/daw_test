#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

out vec2 UV;

uniform mat4 MVP;

void main() {

    gl_Position =
    MVP *
    vec4( pos.x, 1, pos.y, 1);
    UV = uv;
    //vec4(pos.x, 1.0, pos.y, 1.0);

    //fragColor.x = gl_Position.x;
    //fragColor.y = gl_Position.y;
    //fragColor.z = gl_Position.z;
}
