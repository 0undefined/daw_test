#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vNorm;

out vec2 UV;
out vec3 FragmentPos;
out vec3 Normal;

uniform mat4 MVP;
uniform mat4 modelPosition;

void main() {

    gl_Position = MVP * vec4(vertexPosition_modelspace, 1);

    UV = vertexUV;
    FragmentPos = vec3(modelPosition * vec4(vertexPosition_modelspace,1));
    Normal = vNorm;
}
