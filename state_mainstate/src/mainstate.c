#include <daw/daw.h>
#include <daw/utils.h>
#include <daw/logging.h>
#include <daw/rendering.h>
#include <states/mainstate.h>
#include <daw/state.h>
#include <daw/window.h>
#include <glad/gl.h>
#include <worldgen.h>
#include <crate.h>

#define FOV_ORTHO 1

#ifdef FOV_ORTHO
#define FOV_MAX 85
#define FOV_MIN 5
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
  MyQuadTexture,
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

#define SPEED_MIN 16.f
#define SPEED_MAX 128.f
#define CAM_TRANSITION_DT 0.45f

#define COUNT(a) sizeof(a) / sizeof(a[0])
/* this is an unfortunate way of declaring this, unfortunately we _really_ don't
 * want to force compile-time known sizes of the data */
/* The order here matters tremendously, as they're passed to the shader pipeline
 * in the same order. I've wasted many hours due to IBO being passed before
 * other data:) */
#define staticdraw ShaderBuffer_AccessFrequency_static | ShaderBuffer_AccessType_draw
ShaderBuffer shaderbuf[] = {
  SHADERBUFFER_NEW(f32, COUNT(crate),                3, crate,                staticdraw | ShaderBuffer_Type_vertexPosition),
  SHADERBUFFER_NEW(f32, COUNT(crate_texture_coords), 2, crate_texture_coords, staticdraw),
  SHADERBUFFER_NEW(f32, COUNT(crate_normals),        3, crate_normals,        staticdraw),
};
ShaderBuffer shaderbuf2[] = {
  SHADERBUFFER_NEW(f32, COUNT(crate),                 3, crate,                 staticdraw | ShaderBuffer_Type_vertexPosition),
  SHADERBUFFER_NEW(f32, COUNT(crate_texture_coords2), 2, crate_texture_coords2, staticdraw),
  SHADERBUFFER_NEW(f32, COUNT(crate_normals),         3, crate_normals,         staticdraw),
};
ShaderBuffer shaderbuf_quad[] = {
  SHADERBUFFER_NEW(f32, COUNT(quad),     2, quad,     staticdraw | ShaderBuffer_Type_vertexPosition),
  SHADERBUFFER_NEW(f32, COUNT(quad_uv),  2, quad_uv,  staticdraw),
  SHADERBUFFER_NEW(u16, COUNT(quad_ibo), 3, quad_ibo, staticdraw | ShaderBuffer_Type_vertexIndex),
};
#undef COUNT

/*
 * https://learnopengl.com/Lighting/Basic-Lighting
 *  We were just adding some lighting and stuff:)
 *
 *  We need to add uniforms: modelposition (see the transform thingymajig) and
 *  lights.
 *
 *
 *
 *
 *
 * */

    //Lerp: a + f * (b - a);
    // or:  a * (1-f) + f*b;
#define FF(_state) ((_state->fov - FOV_MIN) / (FOV_MAX - FOV_MIN))
#define SPEED(_state) (SPEED_MIN + FF(_state) * (SPEED_MAX - SPEED_MIN))
//#define SPEED(_state) (SPEED_MIN * (1 - FF(_state)) + (FF(_state) * SPEED_MAX))

#define ACCELERATE( x, y, z ) \
  glm_vec3_add((vec3){x, y, z}, s->cam_acc, s->cam_acc)
void move_cam_left(mainstate_state *s)       { ACCELERATE(-SPEED(s),    0,  0); }
void move_cam_left_stop(mainstate_state *s)  { ACCELERATE(+SPEED(s),    0,  0); }
void move_cam_right(mainstate_state *s)      { ACCELERATE( SPEED(s),    0,  0); }
void move_cam_right_stop(mainstate_state *s) { ACCELERATE(-SPEED(s),    0,  0); }
void move_cam_fwd(mainstate_state *s)        { ACCELERATE(     0,        0, -SPEED(s)    ); }
void move_cam_fwd_stop(mainstate_state *s)   { ACCELERATE(     0,        0, +SPEED(s)    ); }
void move_cam_bck(mainstate_state *s)        { ACCELERATE(     0,        0,  SPEED(s)    ); }
void move_cam_bck_stop(mainstate_state *s)   { ACCELERATE(     0,        0, -SPEED(s)    ); }

void move_cam_up(mainstate_state *s)         { ACCELERATE( 0,        SPEED(s),          0); }
void move_cam_up_stop(mainstate_state *s)    { ACCELERATE( 0,       -SPEED(s),          0); }
void move_cam_dwn(mainstate_state *s)        { ACCELERATE( 0,       -SPEED(s),          0); }
void move_cam_dwn_stop(mainstate_state *s)   { ACCELERATE( 0,       +SPEED(s),          0); }

void window_resize_callback(ivec3* dst, ivec2 src) {
  // Map the resolution 1:1
  glm_ivec2_copy(src, *dst);
  // Alternatively, scale down the buffer to 1:4
  //glm_ivec2_divs(src, 4, *dst);
}

void perspective_update_callback(Camera* dst, void *s, ivec2 src) {
  mainstate_state *state = s;
#ifdef FOV_ORTHO
  r_perspective_ortho(dst, (float)state->fov, src);
#else
  r_perspective(&state->c, (float)state->fov, windowsize);
#endif
}

void perspective_update(mainstate_state *s) {
  ivec2 windowsize;
  window_get_size(&windowsize);
  //extern Instance* p;
#ifdef FOV_ORTHO
  r_perspective_ortho(&s->c, (float)s->fov, windowsize);
#else
  r_perspective(&s->c, (float)s->fov, windowsize);
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
  s->cam_dir_dt = CAM_TRANSITION_DT;
  glm_vec3_rotate(s->cam_dir, -(3.141f / 4.f), GLM_YUP);
  //glm_rotate_at(s->c.per, s->cam_pos, 3.141 / 4., GLM_YUP);
  //glm_rotate(s->c.per, 1, GLM_YUP);
  //perspective_update(s);
    INFO("rotation: %.1f  %.1f  %.1f",
        s->c.dir[0],
        s->c.dir[1],
        s->c.dir[2]);
}

void cam_rotate_r(mainstate_state *s) {
  s->cam_dir_dt = CAM_TRANSITION_DT;
  glm_vec3_rotate(s->cam_dir, 3.141f / 4.f, GLM_YUP);
  //glm_rotate(s->c.per, -(3.141 / 4.), GLM_YUP);
  //perspective_update(s);
    INFO("rotation: %.1f  %.1f  %.1f",
        s->c.dir[0],
        s->c.dir[1],
        s->c.dir[2]);
}

void mainstate_init(Window *restrict w, mainstate_state *state, void* arg) {
	INFO("Starting mainstate");

  if (arg != NULL) {
    INFO("Arg was not null!");
  }

  // ----------- Calculate normals TODO MOVE TO ENGINE
  for (usize i = 0; i < sizeof(crate) / sizeof(crate[0]) / 3; i++) {
    // Face index
    const usize f = i / 3;
    // Face offset
    const usize offset = f * 9; // 3 floats pr. vertex, 3 vertices per face.
    vec3 A, B, C;
    glm_vec3_copy(&crate[offset + ((i + 0) % 3) * 3], A);
    glm_vec3_copy(&crate[offset + ((i + 1) % 3) * 3], B);
    glm_vec3_copy(&crate[offset + ((i + 2) % 3) * 3], C);

    // We could allocate vec3's for AB, AC, and the norm, but this is more memory friendly:)
    glm_vec3_sub(A,B,B);
    glm_vec3_sub(A,C,C);

    glm_vec3_cross(B,C,A);
    glm_vec3_normalize(A);

    glm_vec3_copy(A, &crate_normals[i * 3]);
  }

  // Use the INDICES of the assets to specify the shaders to be composed
  static u32 default_shader[] = {MyVertexShader, MyFragmentShader};
  static u32 dither_shader[]  = {MyVertexShader, MyDitherFragShader};
  static u32 quad_shader[]    = {MyQuadVertexShader, MyQuadFragShader};

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
      [MyTexture] = Declare_Texture("resources/atlas.png"),
      [MyQuadTexture] = Declare_Texture("resources/quad.png"),
      [MyGrass] = Declare_Texture("resources/grass.png"),
      [MyStone] = Declare_Texture("resources/stone.png"),

      [MyQuadVertexShader] = Declare_Shader("resources/quad.vert"),
      [MyQuadFragShader] = Declare_Shader("resources/quad.frag"),
      [MyQuadShader] = Declare_ShaderProgram(
          quad_shader, sizeof(quad_shader) / sizeof(quad_shader[0])),
  };

  /// Setup the camera
  // Set the position (it is zero initialized)
  glm_vec3_copy((vec3){WORLD_WIDTH / 2.f, WORLD_HEIGHT / 4.f + 4.f, WORLD_LENGTH / 2.f}, state->c.pos);

  // Copy to the desired position
  glm_vec3_copy(state->c.pos, state->cam_pos);

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
      get_asset(&state->resources, MyDefaultShader),
      // Texture
      ((Texture*)get_asset(&state->resources, MyTexture))->id,
      // Shader
      shaderbuf,
      sizeof(shaderbuf) / sizeof(ShaderBuffer)
      );
  state->objects[1] = RenderObject_new(
      // Shader
      get_asset(&state->resources, MyDefaultShader),
      // Texture
      ((Texture*)get_asset(&state->resources, MyTexture))->id,
      // Shader, using the other UVs
      shaderbuf2,
      sizeof(shaderbuf2) / sizeof(ShaderBuffer)
      );
  state->objects[2] = RenderObject_new(
      // Shader
      get_asset(&state->resources, MyQuadShader),
      // Texture
      ((Texture*)get_asset(&state->resources, MyQuadTexture))->id,
      // Vertices
      shaderbuf_quad,
      sizeof(shaderbuf_quad) / sizeof(ShaderBuffer)
      );

  // ### TEST RENDER BATCH
  if (renderbatch_new(&(state->terrain), 0)) {
    ERROR("Failed to create render batch!");
    exit(EXIT_FAILURE);
  }

  gen_terrain(state->world, WORLD_HEIGHT/2, WORLD_LENGTH, WORLD_WIDTH);
  for (isize i = 0; i < WORLD_SIZE; i++) {
    if (state->world[i] == BLOCK_none) continue;

    const isize y = i / (WORLD_LENGTH * WORLD_WIDTH); // height
    const isize z = (i - (WORLD_LENGTH * WORLD_WIDTH * y)) / WORLD_WIDTH; // length
    const isize x = i % WORLD_WIDTH; // width
    Transform t = {
      .position = {(float)x, (float)y, (float)z},
    };
    switch (state->world[i]) {
      case BLOCK_grass:
        if (-1 == renderbatch_add(&(state->terrain), &(state->objects[0]), &t)) {
          ERROR("Failed to add model 0 to render batch!");
          exit(EXIT_FAILURE);
        }
        break;
      case BLOCK_rock:
        if (-1 == renderbatch_add(&(state->terrain), &(state->objects[1]), &t)) {
          ERROR("Failed to add model 1 to render batch!");
          exit(EXIT_FAILURE);
        }
        break;
      default: break;
    }
  }

  // Create the render object from the buffer
  renderbatch_refresh(&(state->terrain));
  state->terrain.renderobj = RenderObject_new(
      // Shader
      get_asset(&state->resources, MyDefaultShader),
      // Texture
      ((Texture*)get_asset(&state->resources, MyTexture))->id,
      // Vertices
      state->terrain.renderobj.buffer,
      state->terrain.renderobj.buffer_len
      );
  // ### END OF TEST


  // Setup controls
	state->input_bindings[ 0] = BindState(KEY_A, 0, move_cam_left,  move_cam_left_stop);
	state->input_bindings[ 1] = BindState(KEY_D, 0, move_cam_right, move_cam_right_stop);
	state->input_bindings[ 2] = BindState(KEY_W, 0, move_cam_fwd,   move_cam_fwd_stop);
	state->input_bindings[ 3] = BindState(KEY_S, 0, move_cam_bck,   move_cam_bck_stop);
	state->input_bindings[ 4] = BindState(KEY_SPACE, 0, move_cam_up,    move_cam_up_stop);
	state->input_bindings[ 5] = BindState(MOD_CTRL, 0, move_cam_dwn,   move_cam_dwn_stop);

	state->input_bindings[ 6] = BindAction(KEY_MINUS, 0, fov_increment);
	state->input_bindings[ 7] = BindAction(KEY_EQUAL, 0, fov_decrement);

	state->input_bindings[ 8] = BindAction(KEY_Q, 0, cam_rotate_l);
	state->input_bindings[ 9] = BindAction(KEY_E, 0, cam_rotate_r);

	state->input_ctx = (i_ctx){
		.bindings = (binding_t*)&state->input_bindings,
		.len = sizeof(state->input_bindings) / sizeof(binding_t),
	};

	WARN("Number of bindings: %lu", state->input_ctx.len);
	i_ctx_push(&state->input_ctx);



  u32 t[] = {
    BUFFERPARAMETER_SET_PARAMETER(BUFFERPARAMETER_SET_TYPE(0, BufferType_texture), BUFFERPARAMETER_FMT_RGB8),

    // The depth buffer could also be a texture like so:
    //   BUFFERPARAMETER_SET_PARAMETER( BUFFERPARAMETER_SET_TYPE(0, BufferType_texture), BUFFERPARAMETER_FMT_DEPTH32),
    BUFFERPARAMETER_SET_PARAMETER(BUFFERPARAMETER_SET_TYPE(0, BufferType_render), BUFFERPARAMETER_FMT_DEPTH32F),
  };
  FramebufferParameters p[] = {
    // 16 by 16 is just some bogus values, but they cannot be zero, as they're
    //    needed to be set when resetting cameras.
    {.num_textures = 1, .num_renderbuffers = 1, .dimensions = {48, 48, 0}},
  };

  // There's a strange duality between using functions to change render_targets,
  // and manipulating the datastructures directly.
  window_init_renderstack(w, 1, sizeof(t) / sizeof(t[0]), p, t);
  w->render_targets->camera_reset_callback[0] = &perspective_update_callback;
  w->render_targets->framebuffer_size_callback[0] = &window_resize_callback;
  r_set_camera(w->render_targets, 0, &state->c);

}

void* mainstate_free(mainstate_state *state) {
	i_ctx_pop();
  return NULL;
}

StateType mainstate_update(Window *restrict w, mainstate_state *state, f64 dt) {
	StateType next_state = STATE_null;

  // Convert to seconds
  const f32 dsec = (f32)(dt / 1000000.f);

  r_clear_buffer(w->context, w->render_targets, 0);

  //extern Instance* p;
  draw_model(w, 0, &state->terrain.renderobj, (vec4){0,0,0,1});
  //engine_draw_model(&(state->objects[2]), (vec3){0,3,0});
  //engine_draw_model(&(state->objects[0]), (vec3){0,0,0});
  //engine_draw_model(&(state->objects[1]), (vec3){0,0,0});

  // Move the camera
  // ... all of this should be easily selectable in the engine
  // base camera friction of "16" seems good
  const f32 friction = 1.f / (1.f + (dsec * 16.f));
  vec3 acc;
  vec3 speed;

  // Scale acceleration and speed by dsec
  glm_vec3_scale(state->cam_acc,   dsec, acc);
  glm_vec3_scale(state->cam_speed, dsec, speed);

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
  if (state->cam_dir_dt - dsec > 0) {
    //Lerp: a + f * (b - a);
    // or:  a * (1-f) + f*b;
    state->cam_dir_dt -= dsec;
    vec3 a, b;
    const f32 f = 1.f - state->cam_dir_dt / CAM_TRANSITION_DT;
    glm_vec3_copy(state->c.dir, a);
    glm_vec3_copy(state->cam_dir, b);

    a[0] = a[0] * (1.f - f) + f*b[0];
    a[1] = a[1] * (1.f - f) + f*b[1];
    a[2] = a[2] * (1.f - f) + f*b[2];

    glm_vec3_copy(a, state->c.dir);
  }

  return next_state;
}
