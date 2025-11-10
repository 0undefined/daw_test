#version 330 core

in vec2 UV;

out vec3 color;

uniform sampler2D textureSampler;

void main() {
  vec3 c = texture(textureSampler, UV).rgb;
  color = vec3(
      c.x / 2 + UV.x / 2,
      c.y / 2 + UV.y / 2,
      c.z);
}
