#include <daw/types.h>
static const f32 px = (float)(1. / 96.);

static f32 quad[8] = {
  -1.f, -1.f,
   1.f, -1.f,
   1.f,  1.f,
  -1.f,  1.f,
};

static u16 quad_ibo[6] = {
  0, 1, 2,
  2, 3, 0,
};

static f32 quad_uv[8] = {
  0.f,  0.f,
  1.f,  0.f,
  1.f,  1.f,
  0.f,  1.f,
};

static f32 crate_normals[36*3];

static f32 crate_texture_coords[36*2] = {
    // BEHIND 0
    48.f*px, 1.0f,
    64.f*px, 1.0f,
    64.f*px, 0.5f,

    // BEHIND 1
    48.f*px, 1.0f,
    64.f*px, 0.5f,
    48.f*px, 0.5f,

    // REAL LEFT 0
    32.f*px, 0.5f,
    48.f*px, 1.0f,
    48.f*px, 0.5f,

    // REAL LEFT 1
    32.f*px, 0.5f,
    32.f*px, 1.0f,
    48.f*px, 1.0f,

    // BOTTOM 0
    80.f*px, 0.5f,
    96.f*px, 1.0f,
    96.f*px, 0.5f,

    // BOTTOM 1
    80.f*px, 0.5f,
    80.f*px, 1.0f,
    96.f*px, 1.0f,

    // RIGHT 0
    16.f*px, 0.5f,
    32.f*px, 1.0f,
    32.f*px, 0.5f,

    // RIGHT 1
    32.f*px, 1.0f,
    16.f*px, 0.5f,
    16.f*px, 1.0f,

    // TOP 0
    64.f*px, 1.0f,
    80.f*px, 1.0f,
    80.f*px, 0.5f,

    // TOP 1
    64.f*px, 1.0f,
    80.f*px, 0.5f,
    64.f*px, 0.5f,

    // LEFT 0
    0.0f,    0.5f,
    0.0f,    1.0f,
    16.f*px, 1.0f,

    // LEFT 1
    16.f*px, 0.5f,
     0.f*px, 0.5f,
    16.f*px, 1.0f,
};


static f32 crate_texture_coords2[36*2] = {
    // BEHIND 0
    48.f*px, 0.5f,
    64.f*px, 0.5f,
    64.f*px, 0.0f,

    // BEHIND 1
    48.f*px, 0.5f,
    64.f*px, 0.0f,
    48.f*px, 0.0f,

    // REAL LEFT 0
    32.f*px, 0.0f,
    48.f*px, 0.5f,
    48.f*px, 0.0f,

    // REAL LEFT 1
    32.f*px, 0.0f,
    32.f*px, 0.5f,
    48.f*px, 0.5f,

    // BOTTOM 0
    80.f*px, 0.0f,
    96.f*px, 0.5f,
    96.f*px, 0.0f,

    // BOTTOM 1
    80.f*px, 0.0f,
    80.f*px, 0.5f,
    96.f*px, 0.5f,

    // RIGHT 0
    16.f*px, 0.0f,
    32.f*px, 0.5f,
    32.f*px, 0.0f,

    // RIGHT 1
    32.f*px, 0.5f,
    16.f*px, 0.0f,
    16.f*px, 0.5f,

    // TOP 0
    64.f*px, 0.5f,
    80.f*px, 0.5f,
    80.f*px, 0.0f,

    // TOP 1
    64.f*px, 0.5f,
    80.f*px, 0.0f,
    64.f*px, 0.0f,

    // LEFT 0
    0.0f,    0.0f,
    0.0f,    0.5f,
    16.f*px, 0.5f,

    // LEFT 1
    16.f*px, 0.0f,
     0.f*px, 0.0f,
    16.f*px, 0.5f,
};

static f32 crate[36*3] = {
  -0.5f, -0.5f, -0.5f, // 1 -x
  -0.5f, -0.5f,  0.5f,
  -0.5f,  0.5f,  0.5f,
  -0.5f, -0.5f, -0.5f, // 2 -x
  -0.5f,  0.5f,  0.5f,
  -0.5f,  0.5f, -0.5f,
   0.5f,  0.5f, -0.5f, // 3 -z
  -0.5f, -0.5f, -0.5f,
  -0.5f,  0.5f, -0.5f,
   0.5f,  0.5f, -0.5f, // 4 -z
   0.5f, -0.5f, -0.5f,
  -0.5f, -0.5f, -0.5f,
   0.5f, -0.5f,  0.5f, // 5 down
  -0.5f, -0.5f, -0.5f,
   0.5f, -0.5f, -0.5f,
   0.5f, -0.5f,  0.5f, // 6 down
  -0.5f, -0.5f,  0.5f,
  -0.5f, -0.5f, -0.5f,
   0.5f,  0.5f,  0.5f, // 8 +x
   0.5f, -0.5f, -0.5f,
   0.5f,  0.5f, -0.5f,
   0.5f, -0.5f, -0.5f, // 9 +x
   0.5f,  0.5f,  0.5f,
   0.5f, -0.5f,  0.5f,
   0.5f,  0.5f,  0.5f, // 10 up
   0.5f,  0.5f, -0.5f,
  -0.5f,  0.5f, -0.5f,
   0.5f,  0.5f,  0.5f, // 11 up
  -0.5f,  0.5f, -0.5f,
  -0.5f,  0.5f,  0.5f,
  -0.5f,  0.5f,  0.5f, // 7 +z
  -0.5f, -0.5f,  0.5f,
   0.5f, -0.5f,  0.5f,
   0.5f,  0.5f,  0.5f, // 12 +z
  -0.5f,  0.5f,  0.5f,
   0.5f, -0.5f,  0.5f
};
