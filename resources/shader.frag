#version 330 core

// Ouput data
in vec2 UV;
in vec3 FragmentPos;
in vec3 Normal;
out vec3 color;

uniform sampler2D textureSampler;

void main() {
  vec3 light_color_ambient = vec3(0.5, 0.5, 1.0);
  vec3 light_color_diffuse = vec3(0.8, 1.0, 1.0);

  vec3 lightpos = vec3(8, 15, 12);

  float ambient_strength = 0.5;
  vec3 ambient = ambient_strength * light_color_ambient;

  vec3 norm = normalize(Normal);
  vec3 light_dir = normalize(lightpos - FragmentPos);

  float diff = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = diff * light_color_diffuse;
  color = (ambient + diffuse) * texture(textureSampler, UV).rgb;
}
