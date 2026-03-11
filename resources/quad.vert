#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

out vec2 UV;

uniform mat4 MVP;
uniform mat4 modelPosition;

void main() {

    gl_Position =
    MVP *
      //modelPosition *
    vec4(pos.x, pos.y, 0, 1);
    //vec4(pos.x, 1.0, pos.y, 1.0);
    UV = uv;

    //fragColor.x = gl_Position.x;
    //fragColor.y = gl_Position.y;
    //fragColor.z = gl_Position.z;
}
