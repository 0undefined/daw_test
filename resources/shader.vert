#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;

out vec2 UV;

uniform mat4 MVP;

void main() {

    gl_Position = MVP * vec4(vertexPosition_modelspace, 1);

    UV = vertexUV;

    //fragmentcolor.x = vertexPosition_modelspace.x; //= vertexColor;
    //fragmentcolor.y = vertexPosition_modelspace.y; //= vertexColor;
    //fragmentcolor.z = vertexPosition_modelspace.z; //= vertexColor;

    //fragmentcolor += 1;
    //fragmentcolor /= 2;
}
