#include <engine/engine.h>
#include <engine/utils.h>
#include <engine/core/logging.h>
#include <engine/rendering/rendering.h>
#include <states/mainstate.h>
#include <engine/core/state.h>

#define FOV_ORTHO 1

#ifdef FOV_ORTHO
#define FOV_MAX 35
#define FOV_MIN 10
#define FOV_INC 5
#else
#define FOV_MAX 95
#define FOV_MIN 45
#define FOV_INC 20
#endif

enum GameResources {
  MyVertexShader,
  MyFragmentShader,
  MyDefaultShader,
  MyDitherFragShader,
  MyDitherShader,
  MyTexture,
  MyGrass,
  MyStone,

  MyQuadVertexShader,
  MyQuadFragShader,
  MyQuadShader,
};

enum blocktypes {
  Block_air = 0,
  Block_grass,
  Block_stone,
};

#define WORLD_SZ_X 8
#define WORLD_SZ_Y 4
#define WORLD_SZ_Z 8

static const f32 speed = 96.f;

static f32 crate_texture_coords[36*2];
static f32 crate[36*3];
static f32 crate_normals[36*2];
static f32 quad[8];
static u16 quad_ibo[6];

#define COUNT(a) sizeof(a) / sizeof(a[0])
/* this is an unfortunate way of declaring this, unfortunately we _really_ don't
 * want to force compile-time known sizes of the data */
#define staticdraw ShaderBuffer_AccessFrequency_static | ShaderBuffer_AccessType_draw
ShaderBuffer shaderbuf[] = {
  SHADERBUFFER_NEW(f32, COUNT(crate),                3, crate,                staticdraw | ShaderBuffer_Type_vertexPosition),
  SHADERBUFFER_NEW(f32, COUNT(crate_texture_coords), 2, crate_texture_coords, staticdraw),
  SHADERBUFFER_NEW(f32, COUNT(crate_normals),        2, crate_normals,        staticdraw),
};
ShaderBuffer shaderbuf2[] = {
  SHADERBUFFER_NEW(f32, COUNT(crate),                3, crate,                staticdraw | ShaderBuffer_Type_vertexPosition),
  SHADERBUFFER_NEW(f32, COUNT(crate_texture_coords), 2, crate_texture_coords, staticdraw),
  SHADERBUFFER_NEW(f32, COUNT(crate_normals),        2, crate_normals,        staticdraw),
};
ShaderBuffer shaderbuf_quad[] = {
  SHADERBUFFER_NEW(f32, COUNT(quad),                2, quad,                  staticdraw | ShaderBuffer_Type_vertexPosition),
  SHADERBUFFER_NEW(u16, COUNT(quad_ibo),            3, quad_ibo,              staticdraw | ShaderBuffer_Type_vertexIndex),
};
#undef COUNT


#define ACCELERATE( x, y, z ) \
  glm_vec3_add((vec3){x, y, z}, s->cam_acc, s->cam_acc)
void move_cam_left(mainstate_state *s)       { ACCELERATE(-speed,    0,  0); }
void move_cam_left_stop(mainstate_state *s)  { ACCELERATE(+speed,    0,  0); }
void move_cam_right(mainstate_state *s)      { ACCELERATE( speed,    0,  0); }
void move_cam_right_stop(mainstate_state *s) { ACCELERATE(-speed,    0,  0); }
void move_cam_fwd(mainstate_state *s)        { ACCELERATE(     0,        0, -speed    ); }
void move_cam_fwd_stop(mainstate_state *s)   { ACCELERATE(     0,        0, +speed    ); }
void move_cam_bck(mainstate_state *s)        { ACCELERATE(     0,        0,  speed    ); }
void move_cam_bck_stop(mainstate_state *s)   { ACCELERATE(     0,        0, -speed    ); }

void move_cam_up(mainstate_state *s)         { ACCELERATE( 0,        speed,          0); }
void move_cam_up_stop(mainstate_state *s)    { ACCELERATE( 0,       -speed,          0); }
void move_cam_dwn(mainstate_state *s)        { ACCELERATE( 0,       -speed,          0); }
void move_cam_dwn_stop(mainstate_state *s)   { ACCELERATE( 0,       +speed,          0); }

void perspective_update(mainstate_state *s) {
#ifdef FOV_ORTHO
  r_perspective_ortho(s->fov, &s->c);
#else
  r_perspective(s->fov, &s->c);
#endif
}

void fov_increment(mainstate_state *s) {
  if (s->fov >= FOV_MAX) {
    s->fov = FOV_MAX;
  } else {
    s->fov = s->fov + FOV_INC;
    WARN("changing fov: to %.1f", s->fov);
  }
  perspective_update(s);
}

void fov_decrement(mainstate_state *s) {
  if (s->fov <= FOV_MIN) {
    s->fov = FOV_MIN;
  } else {
    s->fov = s->fov - FOV_INC;
    WARN("changing fov: to %.1f", s->fov);
  }
  perspective_update(s);
}

void cam_rotate_l(mainstate_state *s) {
  s->cam_dir_dt = 0.300;
  glm_vec3_rotate(s->cam_dir, 3.141 / 4., GLM_YUP);
  //glm_rotate_at(s->c.per, s->cam_pos, 3.141 / 4., GLM_YUP);
  //glm_rotate(s->c.per, 1, GLM_YUP);
  //perspective_update(s);
    INFO("rotation: %.1f  %.1f  %.1f",
        s->c.dir[0],
        s->c.dir[1],
        s->c.dir[2]);
}

void cam_rotate_r(mainstate_state *s) {
  s->cam_dir_dt = 0.300;
  glm_vec3_rotate(s->cam_dir, -(3.141 / 4.), GLM_YUP);
  //glm_rotate(s->c.per, -(3.141 / 4.), GLM_YUP);
  //perspective_update(s);
    INFO("rotation: %.1f  %.1f  %.1f",
        s->c.dir[0],
        s->c.dir[1],
        s->c.dir[2]);
}

void mainstate_init(mainstate_state *state, void* arg) {
	INFO("Starting mainstate");

  if (arg != NULL) {
    INFO("Arg was not null!");
  }

  // Use the INDICES of the assets to specify the shaders to be composed
  static u32 default_shader[] = {MyVertexShader, MyFragmentShader};
  static u32 dither_shader[]  = {MyVertexShader, MyDitherFragShader};
  static u32 quad_shader[]  =   {MyQuadVertexShader, MyQuadFragShader};

  /* 0. Declare resources */
  static asset_t mainstate_assets[] = {
      [MyVertexShader] = Declare_Shader("resources/shader.vert"),
      [MyFragmentShader] = Declare_Shader("resources/shader.frag"),
      ///* It is important to list shader program last, and at the moment, you
      // * need to make duplicate shader declarations for each dependency if it is
      // * duplicated. */
      [MyDefaultShader] = Declare_ShaderProgram(
          default_shader, sizeof(default_shader) / sizeof(default_shader[0])),

      [MyDitherFragShader] = Declare_Shader("resources/dither.frag"),
      [MyDitherShader] = Declare_ShaderProgram(
          dither_shader, sizeof(dither_shader) / sizeof(dither_shader[0])),
      [MyTexture] = Declare_Texture("resources/texture.png"),
      [MyGrass] = Declare_Texture("resources/grass.png"),
      [MyStone] = Declare_Texture("resources/stone.png"),

      [MyQuadVertexShader] = Declare_Shader("resources/quad.vert"),
      [MyQuadFragShader] = Declare_Shader("resources/quad.frag"),
      [MyQuadShader] = Declare_ShaderProgram(
          quad_shader, sizeof(quad_shader) / sizeof(quad_shader[0])),
  };

  /// Setup the camera
  // Set the position (it is zero initialized)
  //glm_vec3_copy((vec3){4,3,4}, state->c.pos);
  glm_vec3_copy((vec3){WORLD_SZ_X / 2.f, WORLD_SZ_Y / 2.f + 4.f, WORLD_SZ_Z / 2.f}, state->c.pos);

  // Copy to the desired position
  glm_vec3_copy(state->c.pos, state->cam_pos);
  //glm_vec3_copy((vec3){0,0,0}, state->cam_speed);
  //glm_vec3_copy((vec3){0,0,0}, state->cam_acc);

  // Field of view
  state->fov = FOV_MIN + 2 * FOV_INC;

  // Set the viewing angle of the camera -- the direction is subtracted from the
  // position.
  // The actual direction of the camera.
  glm_vec3_copy((vec3){1, 2, 1}, state->c.dir);
  glm_vec3_normalize(state->c.dir);
  // Copy to the desired direction.
  glm_vec3_copy(state->c.dir, state->cam_dir);

  perspective_update(state);
  r_set_camera(&state->c);

  INFO("initial position: %.1f  %.1f  %.1f",
      state->c.pos[0],
      state->c.pos[1],
      state->c.pos[2]);

  state->resources.assets = mainstate_assets;
  state->resources.assets_len = sizeof(mainstate_assets) / sizeof(mainstate_assets[0]);

  /* 1. Load resources */
  resources_load(&state->resources);

  // TODO: Fixup this mess below:


  // Create render object for the two models.
  // Use the same mesh & UV
  state->objects[0] = RenderObject_new(
      // Shader
      get_asset(&state->resources, MyDitherShader),
      // Texture
      ((Texture*)get_asset(&state->resources, MyStone))->id,
      // Vertices
      shaderbuf,
      sizeof(shaderbuf) / sizeof(ShaderBuffer)
      );
  state->objects[1] = RenderObject_new(
      // Shader
      get_asset(&state->resources, MyDefaultShader),
      // Texture
      ((Texture*)get_asset(&state->resources, MyGrass))->id,
      // Vertices
      shaderbuf2,
      sizeof(shaderbuf2) / sizeof(ShaderBuffer)
      );
  state->objects[2] = RenderObject_new(
      // Shader
      get_asset(&state->resources, MyQuadShader),
      // Texture
      0,
      // Vertices
      shaderbuf_quad,
      sizeof(shaderbuf_quad) / sizeof(ShaderBuffer)
      );

  // ### TEST RENDER BATCH
  if (renderbatch_new(&(state->terrain), 0)) {
    ERROR("Failed to create render batch!");
    exit(EXIT_FAILURE);
  }
  for (int y = -16; y < 16; y++) {
  for (int x = -16; x < 16; x++) {
    Transform t = {
      .position = {2 * x, 0, 2 * y},
    };
    if (-1 == renderbatch_add(&(state->terrain), &(state->objects[1]), &t)) {
      ERROR("Failed to add model2 to render batch!");
      exit(EXIT_FAILURE);
    }
  }
  }
  for (int x = -4; x < 4; x++) {
    Transform t = {
      .position = {2 * x, 2, 8},
    };
    if (-1 == renderbatch_add(&(state->terrain), &(state->objects[0]), &t)) {
      ERROR("Failed to add model2 to render batch!");
      exit(EXIT_FAILURE);
    }
  }
  for (int x = -4; x < 4; x++) {
    Transform t = {
      .position = {2 * x, 2, -8},
    };
    if (-1 == renderbatch_add(&(state->terrain), &(state->objects[0]), &t)) {
      ERROR("Failed to add model2 to render batch!");
      exit(EXIT_FAILURE);
    }
  }

  // Create the render object from the buffer
  renderbatch_refresh(&(state->terrain));
  state->terrain.renderobj = RenderObject_new(
      // Shader
      get_asset(&state->resources, MyDitherShader),
      // Texture
      ((Texture*)get_asset(&state->resources, MyGrass))->id,
      // Vertices
      state->terrain.renderobj.buffer,
      state->terrain.renderobj.buffer_len
      );
  // ### END OF TEST


  // Setup controls
	state->input_bindings[ 0] = BindState(/*'A'*/ 30, 0, move_cam_left,  move_cam_left_stop);
	state->input_bindings[ 1] = BindState(/*'D'*/ 32, 0, move_cam_right, move_cam_right_stop);
	state->input_bindings[ 2] = BindState(/*'W'*/ 17, 0, move_cam_fwd,   move_cam_fwd_stop);
	state->input_bindings[ 3] = BindState(/*'S'*/ 31, 0, move_cam_bck,   move_cam_bck_stop);
	state->input_bindings[ 4] = BindState(/*'W'*/ 57, 0, move_cam_up,    move_cam_up_stop);
	state->input_bindings[ 5] = BindState(/*'S'*/ 29, 0, move_cam_dwn,   move_cam_dwn_stop);

	state->input_bindings[ 6] = BindAction(/*'-'*/ 13, 0, fov_increment);
	state->input_bindings[ 7] = BindAction(/*'='*/ 12, 0, fov_decrement);

	state->input_bindings[ 8] = BindAction(/*'Q'*/ 16, 0, cam_rotate_l);
	state->input_bindings[ 9] = BindAction(/*'E'*/ 18, 0, cam_rotate_r);

	state->input_ctx = (i_ctx){
		.bindings = (binding_t*)&state->input_bindings,
		.len = sizeof(state->input_bindings) / sizeof(binding_t),
	};

	WARN("Number of bindings: %lu", state->input_ctx.len);
	i_ctx_push(&state->input_ctx);
}

void* mainstate_free(mainstate_state *state) {
	i_ctx_pop();
  return NULL;
}

StateType mainstate_update(f64 dt, mainstate_state *state) {
	StateType next_state = STATE_null;

  // Convert to seconds
  dt = dt / 1000000.0;

  engine_draw_model(&state->terrain.renderobj, (vec3){0,0,0});
  engine_draw_model(&(state->objects[2]), (vec3){0,3,0});
  //engine_draw_model(&(state->objects[0]), (vec3){0,0,0});
  //engine_draw_model(&(state->objects[1]), (vec3){0,0,0});

  // Move the camera
  // ... all of this should be easily selectable in the engine
  const float friction = 1.f / (1.f + (dt * 7.5f));
  vec3 acc;
  vec3 speed;

  // Scale acceleration and speed by dt
  glm_vec3_scale(state->cam_acc,   dt, acc);
  glm_vec3_scale(state->cam_speed, dt, speed);

  glm_vec3_add(acc,   speed, speed);

  // Add delta speed to speed
  glm_vec3_add(acc,   state->cam_speed, state->cam_speed);
  // Scale by friction
  glm_vec3_scale(state->cam_speed, friction, state->cam_speed);

  // add speed to position
  // Move it along the actual direction of the camera
  vec3 dir;
  glm_vec3_copy(state->c.dir, dir);

  // Let P_0 be the new position, P the old position, s the delta movement, and
  // D the direction vector, then
  //   P_0 = ( P.x + D.y * s.x + D.x * s.y
  //         , P.y + D.y * s.y - D.x * s.x)

  vec3 t;
  // Copy vertical momentum
  t[1] = speed[1];
  // Calculate perpendicular (to camera direction) movement
  t[0] = dir[2] * speed[0];
  t[2] = dir[2] * speed[2];

  t[0] += dir[0] * speed[2];
  t[2] -= dir[0] * speed[0];

  // Update position
  glm_vec3_add(state->cam_pos,   t, state->cam_pos);

  // Update the actual camera position
  glm_vec3_copy(state->cam_pos,  state->c.pos);

  // Rotate the camera
  if (state->cam_dir_dt - dt > 0) {
    //Lerp: a + f * (b - a);
    // or:  a * (1-f) + f*b;
    const f32 lerpduration = 0.300;
    state->cam_dir_dt -= dt;
    vec3 a, b;
    const f32 f = 1.0 - state->cam_dir_dt / lerpduration;
    glm_vec3_copy(state->c.dir, a);
    glm_vec3_copy(state->cam_dir, b);

    a[0] = a[0] * (1.0 - f) + f*b[0];
    a[1] = a[1] * (1.0 - f) + f*b[1];
    a[2] = a[2] * (1.0 - f) + f*b[2];

    glm_vec3_copy(a, state->c.dir);
  }

  return next_state;
}

static const f32 px = 1. / 96.;

static f32 quad[] = {
  -1.f, -1.f,
   1.f, -1.f,
   1.f,  1.f,
  -1.f,  1.f,
};

static u16 quad_ibo[] = {
  0, 1, 2,
  2, 3, 0,
};

static f32 crate_texture_coords[] = {
    // BEHIND 0
    49.f*px, 1.0f,
    65.f*px, 1.0f,
    65.f*px, 0.0f,

    // REAL LEFT 0
    33.f*px, 0.0f,
    49.f*px, 1.0f,
    49.f*px, 0.0f,

    // BOTTOM 0
    81.f*px, 0.f,
    96.f*px, 1.f,
    96.f*px, 0.f,

    // REAL LEFT 1
    33.f*px, 0.0f,
    33.f*px, 1.0f,
    49.f*px, 1.0f,

    // BEHIND 1
    49.f*px, 1.0f,
    65.f*px, 0.0f,
    49.f*px, 0.0f,

    // BOTTOM 1
    81.f*px, 0.f,
    81.f*px, 1.f,
    96.f*px, 1.f,

    // LEFT 0
    0.0f, 0.0f,
    0.0f, 1.0f,
    17.f*px, 1.0f,

    // RIGHT 0
    17.f*px, 0.0f,
    33.f*px, 1.0f,
    33.f*px, 0.0f,

    // RIGHT 1
    33.f*px, 1.0f,
    17.f*px, 0.0f,
    17.f*px, 1.0f,

    // TOP 0
    80.f*px, 1.0f,
    65.f*px, 1.0f,
    65.f*px, 0.0f,

    // TOP 1
    65.f*px, 1.f,
    80.f*px, 0.f,
    65.f*px, 0.f,

    // LEFT 1
    17.f*px, 0.0f,
     0.f*px, 0.0f,
    17.f*px, 1.0f,
};

static f32 crate_normals[] = {
    // BEHIND 0
    49.f*px, 1.0f,
    65.f*px, 1.0f,
    65.f*px, 0.0f,

    // REAL LEFT 0
    33.f*px, 0.0f,
    49.f*px, 1.0f,
    49.f*px, 0.0f,

    // BOTTOM 0
    81*px, 0,
    96*px, 1,
    96*px, 0,

    // REAL LEFT 1
    33.f*px, 0.0f,
    33.f*px, 1.0f,
    49.f*px, 1.0f,

    // BEHIND 1
    49.f*px, 1.0f,
    65.f*px, 0.0f,
    49.f*px, 0.0f,

    // BOTTOM 1
    81*px, 0,
    81*px, 1,
    96*px, 1,

    // LEFT 0
    0.0f, 0.0f,
    0.0f, 1.0f,
    17.f*px, 1.0f,

    // RIGHT 0
    17.f*px, 0.0f,
    33.f*px, 1.0f,
    33.f*px, 0.0f,

    // RIGHT 1
    33.f*px, 1.0f,
    17.f*px, 0.0f,
    17.f*px, 1.0f,

    // TOP 0
    80.f*px, 1.0f,
    65.f*px, 1.0f,
    65.f*px, 0.0f,

    // TOP 1
    65.f*px, 1.f,
    80.f*px, 0.f,
    65.f*px, 0.f,

    // LEFT 1
    17.f*px, 0.0f,
     0.f*px, 0.0f,
    17.f*px, 1.0f,
};

static f32 crate[] = {
  -1.0f, -1.0f, -1.0f, // triangle 1 : begin
  -1.0f, -1.0f,  1.0f,
  -1.0f,  1.0f,  1.0f, // triangle 1 : end
  1.0f,  1.0f, -1.0f, // triangle 2 : begin
  -1.0f, -1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f, // triangle 2 : end
  1.0f, -1.0f,  1.0f,
  -1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f,  1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f,  1.0f,  1.0f,
  -1.0f,  1.0f, -1.0f,
  1.0f, -1.0f,  1.0f,
  -1.0f, -1.0f,  1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f,  1.0f,  1.0f,
  -1.0f, -1.0f,  1.0f,
  1.0f, -1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f,  1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f, -1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,
  1.0f,  1.0f,  1.0f,
  -1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,
  -1.0f,  1.0f,  1.0f,
  1.0f, -1.0f,  1.0f
};
