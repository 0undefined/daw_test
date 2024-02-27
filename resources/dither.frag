#version 330 core

const float pR = 0.299;
const float pG = 0.587;
const float pB = 0.114;

const vec3 p = vec3(pR, pG, pB);

float dithering_matrix[64];

in vec2 UV;
in vec4 gl_FragCoord;
//in vec2 gl_fragCoord;
out vec4 color;

uniform sampler2D textureSampler;

void main() {
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = UV; //iTime + (fragCoord.xy / iResolution.xy);
    vec2 fragPos = gl_FragCoord.xy;

    vec3 col = texture(textureSampler, UV).rgb;

    vec3 colsqrt = col * col;
    float luminance = sqrt(dot(p, colsqrt));

    dithering_matrix = float[](
       0.0, 48.0, 12.0, 60.0,  3.0, 51.0, 15.0, 61.0,
      32.0, 16.0, 44.0, 28.0, 35.0, 19.0, 47.0, 31.0,
       8.0, 56.0,  4.0, 52.0, 11.0, 59.0,  7.0, 55.0,
      40.0, 24.0, 36.0, 20.0, 43.0, 27.0, 39.0, 23.0,
       2.0, 50.0, 14.0, 62.0,  1.0, 49.0, 13.0, 61.0,
      34.0, 18.0, 46.0, 30.0, 33.0, 17.0, 45.0, 29.0,
      10.0, 58.0,  6.0, 54.0,  9.0, 57.0,  5.0, 53.0,
      42.0, 26.0, 38.0, 22.0, 41.0, 25.0, 37.0, 21.0
    );

    /* BEGIN PASTE */
    int x = int(mod(fragPos.x, 8.0));
    int y = int(mod(fragPos.y, 8.0));

    // index of flat matrix
    int idx = (x * 8) + y;
    float dither = dithering_matrix[idx];
    // normalize the value
    dither *= (1.0 / 64.0);

    /* END PASTE */

    // Output to screen
    if (luminance > dither) {
      color = vec4(col, 1.0);
    } else {
      color = vec4(
          float(0x10) / 255.f, float(0x0a) / 255.f, float(0x33) / 255.f, 1.f
      );

      //color = vec4(0.0);
    }
}
