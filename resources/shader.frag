#version 330 core

// Ouput data
in vec2 UV;
in vec3 FragmentPos;
in vec3 Normal;
out vec3 color;

uniform sampler2D textureSampler;

void main() {
  vec3 light_color_ambient = vec3(0.55, 0.55, 1.0);
  vec3 light_color_diffuse = vec3(1.0, 0.98, 0.84);

  vec3 lightpos = vec3(7, 17, 10);

  float ambient_strength = 0.45;
  vec3 ambient = ambient_strength * light_color_ambient;

  vec3 norm = normalize(Normal);
  vec3 light_dir = normalize(lightpos - FragmentPos);

  float diff = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = diff * light_color_diffuse;
  color = (ambient + diffuse) * texture(textureSampler, UV).rgb;
}
