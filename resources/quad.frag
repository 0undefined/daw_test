#version 330 core

in vec3 fragColor;

out vec4 color;

//uniform sampler2D textureSampler;

void main() {
    color.x = fragColor.x;
    color.y = fragColor.y;
    color.z = fragColor.z;
}
