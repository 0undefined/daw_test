#define CGLM_FORCE_DEPTH_ZERO_TO_ONE 1

#include <daw/daw.h>
#include <daw/utils.h>
#include <daw/logging.h>
#include <daw/rendering.h>
#include <states/mainstate.h>
#include <daw/state.h>
#include <daw/window.h>
#include <glad/gl.h>
#include <crate.h>


#include <assimp/cimport.h>        // Plain-C interface
#include <assimp/scene.h>          // Output data structure
#include <assimp/postprocess.h>    // Post processing flags

#define FOV_ORTHO 1

#ifdef FOV_ORTHO
#define FOV_MAX 145
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
  MyQuadFullscreenVertexShader,
  MyQuadShader,

  MySimpleShader_vert,
  MySimpleShader_frag,
  MySimpleShader,
  MyFullscreenShader,
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
ShaderBuffer shaderbuf_quad_fullscreen[] = {
  SHADERBUFFER_NEW(f32, COUNT(quad_fullscreen), 2, quad_fullscreen,     staticdraw | ShaderBuffer_Type_vertexPosition),
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

void window_resize_callback_ui(ivec3* dst, ivec2 src) {
  // Map the resolution 1:1
  glm_ivec2_copy(src, *dst);
  // Alternatively, scale down the buffer to 1:4
  //glm_ivec2_divs(src, 4, *dst);
}

void window_resize_callback(ivec3* dst, ivec2 src) {
  // Map the resolution 1:1
  glm_ivec2_copy(src, *dst);
  // Alternatively, Supersampling (not working??)
  //glm_ivec2_scale(src, 2, *dst);
  // Alternatively, scale down the buffer to 1:4
  //glm_ivec2_divs(src, 4, *dst);
}

ShaderBuffer model_shaderbuf[3];
ShaderBuffer light_shaderbuf[3];

static void load_model_from_file(const char* file_path, struct assimp_mesh *mesh) {
  // Start the import on the given file with some example postprocessing
  // Usually - if speed is not the most important aspect for you - you'll t
  // probably to request more postprocessing than we do in this example.
  const struct aiScene* scene = aiImportFile( file_path,
    aiProcess_CalcTangentSpace       |
    aiProcess_Triangulate            |
    //aiProcess_JoinIdenticalVertices  |
    aiProcess_SortByPType);

  // If the import failed, report it
  if(scene == NULL) {
    ERROR("Failed to import model: %s", aiGetErrorString());
    return;
  }

  if (mesh == NULL) {
    mesh = calloc(1, sizeof(struct assimp_mesh));
  }

  for (u32 m = 0; m < scene->mNumMeshes; m++) {
    mesh->vertices_len = scene->mMeshes[m]->mNumVertices * 3;
    mesh->normals_len = scene->mMeshes[m]->mNumVertices * 3;
    //mesh_uv_len = scene->mMeshes[m]->mNumVertices * 2;

    mesh->vertices = calloc(scene->mMeshes[m]->mNumVertices * 3, sizeof(float));
    mesh->normals = calloc(scene->mMeshes[m]->mNumVertices * 3, sizeof(float));
    //mesh_uv = calloc(scene->mMeshes[m]->mNumVertices * 2, sizeof(float));
    for (u32 i = 0; i < scene->mMeshes[m]->mNumVertices; i++) {
      mesh->vertices[i * 3 + 0] = scene->mMeshes[m]->mVertices[i].x;
      mesh->vertices[i * 3 + 1] = scene->mMeshes[m]->mVertices[i].y;
      mesh->vertices[i * 3 + 2] = scene->mMeshes[m]->mVertices[i].z;

      mesh->normals[i * 3 + 0] = scene->mMeshes[m]->mNormals[i].x;
      mesh->normals[i * 3 + 1] = scene->mMeshes[m]->mNormals[i].y;
      mesh->normals[i * 3 + 2] = scene->mMeshes[m]->mNormals[i].z;

      //if (scene->mMeshes[m]->mTextureCoords[i] == NULL) {
      //  mesh_uv[i * 2 + 0] = 0;
      //  mesh_uv[i * 2 + 1] = 0;
      //} else {
      //  mesh_uv[i * 2 + 0] = scene->mMeshes[m]->mTextureCoords[i]->x;
      //  mesh_uv[i * 2 + 1] = scene->mMeshes[m]->mTextureCoords[i]->y;
      //}
    }

    // Count the vertices for all faces first:)
    mesh->indices_len = 0;
    for (u32 i = 0; i < scene->mMeshes[m]->mNumFaces; i++) {
      mesh->indices_len += scene->mMeshes[m]->mFaces[i].mNumIndices;
    }

    mesh->indices = calloc(mesh->indices_len, sizeof(float));
    usize mesh_idx = 0;
    for (u32 i = 0; i < scene->mMeshes[m]->mNumFaces; i++) {
      for (u32 j = 0; j < scene->mMeshes[m]->mFaces[i].mNumIndices; j++) {
        mesh->indices[mesh_idx] = scene->mMeshes[m]->mFaces[i].mIndices[j];
        mesh_idx++;
      }
    }

    // normals??
  }
  // Now we can access the file's contents
  //DoTheSceneProcessing( scene);

  // We're done. Release all resources associated with this import
  aiReleaseImport(scene);
}

Camera UI_Camera;
void perspective_update_callback_ui(void *restrict w, Camera *restrict dst, void *s, ivec2 src) {
  mainstate_state *state = s;
  const f32 ratio = (f32)src[0] / (f32)src[1];
  const f32 sz = src[1];
  const f32 ratiow = (f32)src[1] / (f32)src[0];
  const f32 sw = src[0];
  //glm_frustum(-src[0] * ratio, src[0] * ratio, -src[1] * ratio, src[1] * ratio, 0.f, 10.f, dst->per);
  //glm_frustum(-src[0] * ratio, src[0] * ratio, -src[1] * ratio, src[1] * ratio, 0.f, 10.f, dst->per);
  //glm_frustum(-10.f, 10.f, -10.f, 10.f, 1.0f, 100.f, dst->per);
  //r_perspective_ortho(dst, 15, src);
  glm_ortho(-sz * ratio, sz * ratio, -sw * ratiow, sw * ratiow, -sz * 10.f, sz * 10.f, dst->per);
  state->objects[3].texture = ((Window *restrict)w)->render_targets->buffer[1];
}

void perspective_update_callback(void *restrict _ /* unused */, Camera* dst, void *s, ivec2 src) {
  mainstate_state *state = s;
#ifdef FOV_ORTHO
  r_perspective_ortho(dst, (float)state->fov, src);
#else
  r_perspective(dst, (float)state->fov, src);
#endif
}

void perspective_update(mainstate_state *s) {
  ivec2 windowsize;
  window_get_size(&windowsize);

#ifdef FOV_ORTHO
  r_perspective_ortho(&s->c, (float)s->fov, windowsize);
#else
  r_perspective(&s->c, (float)s->fov, windowsize);
#endif
}

void fov_increment(mainstate_state *s) {
  if (s->fov + FOV_INC >= FOV_MAX) {
    s->fov = FOV_MAX;
  } else {
    s->fov = s->fov + FOV_INC;
    WARN("changing fov: to %.1f", s->fov);
  }
  perspective_update(s);
}

void fov_decrement(mainstate_state *s) {
  if (s->fov - FOV_INC <= FOV_MIN) {
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

    glm_vec3_cross(C,B,A);
    glm_vec3_normalize(A);

    glm_vec3_copy(A, &crate_normals[i * 3]);
  }

  // Use the INDICES of the assets to specify the shaders to be composed
  static u32 default_shader[] = {MyVertexShader, MyFragmentShader};
  static u32 dither_shader[]  = {MyVertexShader, MyDitherFragShader};
  static u32 quad_shader[]    = {MyQuadVertexShader, MyQuadFragShader};
  static u32 quad_fullscreen_shader[]    = {MyQuadFullscreenVertexShader, MyQuadFragShader};
  static u32 simple_shader[]    = {MySimpleShader_vert, MySimpleShader_frag};

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
      [MyQuadFullscreenVertexShader] = Declare_Shader("resources/quad_fs.vert"),
      [MyFullscreenShader] = Declare_ShaderProgram(
          quad_fullscreen_shader, sizeof(quad_fullscreen_shader) / sizeof(quad_fullscreen_shader[0])),

      [MySimpleShader_vert] = Declare_Shader("resources/simpl.vert"),
      [MySimpleShader_frag] = Declare_Shader("resources/simpl.frag"),
      [MySimpleShader] = Declare_ShaderProgram(
          simple_shader, sizeof(simple_shader) / sizeof(simple_shader[0])),

  };

  /// Setup the camera
  // Set the position (it is zero initialized)
  glm_vec3_copy((vec3){CHUNK_WIDTH * WORLD_WIDTH / 2.f, CHUNK_HEIGHT / 4.f + 4.f, CHUNK_LENGTH * WORLD_LENGTH / 2.f}, state->c.pos);

  // Copy to the desired position
  glm_vec3_copy(state->c.pos, state->cam_pos);

  // Field of view
  state->fov = FOV_MIN + FOV_INC;

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

  gen_terrain(state->world);
  usize total = 0;
  usize culled = 0;
  for (isize i = 0; i < WORLD_SIZE * CHUNK_SIZE; i++) {
    if (state->world[i] == BLOCK_none) continue;

    const isize chunk_idx = i / CHUNK_SIZE;
    const isize local_idx = i % CHUNK_SIZE;
    const isize local_x = local_idx % CHUNK_WIDTH;
    const isize local_y = local_idx / (CHUNK_LENGTH * CHUNK_WIDTH);
    const isize local_z = (local_idx - (CHUNK_LENGTH * CHUNK_WIDTH * local_y)) / CHUNK_WIDTH;

    const isize chunk_x = chunk_idx % WORLD_WIDTH;
    const isize chunk_z = chunk_idx / WORLD_WIDTH;

    const isize y = local_y; // height
    const isize z = local_z + chunk_z * CHUNK_LENGTH; // length
    const isize x = local_x + chunk_x * CHUNK_WIDTH; // width

    Transform t = {
      .position = {(float)x, (float)y, (float)z},
    };
    if (state->world[i] & ((1 << 5) - 1)) {
      total++;
    }
    if (SURROUNDED(state->world[i])) {
      culled++;
      continue;
    }
    switch (state->world[i] & ((1 << 5) - 1)) {
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

  INFO("Culled %u / %u (%.2f)", culled, total, (float)culled / (float)total);

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

  // Setup flat "final" camera
  ivec2 ws = {200,200};
  glm_vec3_copy((vec3){0,0,0}, UI_Camera.pos);
  glm_vec3_copy((vec3){0,0,1}, UI_Camera.dir);
  r_perspective_ortho(&UI_Camera, 15, ws);


  u32 t[] = {
    // Texture final texture that is presented to screen
    BUFFERPARAMETER_SET_PARAMETER(BUFFERPARAMETER_SET_TYPE(0, BufferType_texture), BUFFERPARAMETER_FMT_RGB8),

    // Target texture for intermmediate FB
    BUFFERPARAMETER_SET_PARAMETER(BUFFERPARAMETER_SET_TYPE(0, BufferType_texture), BUFFERPARAMETER_FMT_RGB8),

    // The depth buffer could also be a texture like so:
    //   BUFFERPARAMETER_SET_PARAMETER( BUFFERPARAMETER_SET_TYPE(0, BufferType_texture), BUFFERPARAMETER_FMT_DEPTH32),
    BUFFERPARAMETER_SET_PARAMETER(BUFFERPARAMETER_SET_TYPE(0, BufferType_render), BUFFERPARAMETER_FMT_DEPTH32F),

  };
  FramebufferParameters p[] = {
    // 16 by 16 is just some bogus values, but they cannot be zero, as they're
    //    needed to be set when resetting cameras.
    {.num_textures = 1, .num_renderbuffers = 0, .dimensions = {48, 48, 0}},
    {.num_textures = 1, .num_renderbuffers = 1, .dimensions = {48, 48, 0}},
  };

  // There's a strange duality between using functions to change render_targets,
  // and manipulating the datastructures directly.
  window_init_renderstack(w, 2, sizeof(t) / sizeof(t[0]), p, t);
  w->render_targets->camera_reset_callback[0] = &perspective_update_callback_ui;
  w->render_targets->framebuffer_size_callback[0] = &window_resize_callback_ui;
  w->render_targets->camera_reset_callback[1] = &perspective_update_callback;
  w->render_targets->framebuffer_size_callback[1] = &window_resize_callback;

  r_set_camera(w->render_targets, 0, &UI_Camera);
  r_set_camera(w->render_targets, 1, &state->c);

  // Remember to keep this texture up-to date when resizing! A caveat I'd rather
  // have implemented in the engine s.t. I don't need to think about it here.
  // I guess it is soon time to define a "scene" structure for DAW.
  state->objects[3] = RenderObject_new(
      // Shader
      get_asset(&state->resources, MyFullscreenShader),
      // Texture
      w->render_targets->buffer[1],
      // Vertices
      shaderbuf_quad_fullscreen,
      sizeof(shaderbuf_quad_fullscreen) / sizeof(ShaderBuffer)
  );


  load_model_from_file("resources/suzanne.obj", &state->meshes[0]);
  load_model_from_file("resources/icosphere.obj", &state->meshes[1]);

  model_shaderbuf[0] =
  SHADERBUFFER_NEW(f32, state->meshes[0].vertices_len, 3, state->meshes[0].vertices, staticdraw | ShaderBuffer_Type_vertexPosition);
  model_shaderbuf[1] =
  SHADERBUFFER_NEW(f32, state->meshes[0].normals_len, 3, state->meshes[0].normals, staticdraw);
  //model_shaderbuf[2] =
  //SHADERBUFFER_NEW(f32, state->meshes[0].uv_len, 2, state->meshes[0].uv, staticdraw);
  model_shaderbuf[2] =
  SHADERBUFFER_NEW(u32, state->meshes[0].indices_len,  3, state->meshes[0].indices,  staticdraw | ShaderBuffer_Type_vertexIndex);

  state->objects[4] = RenderObject_new(
      // Shader
      get_asset(&state->resources, MySimpleShader),
      // Texture
      0, //w->render_targets->buffer[1],
      // Vertices
      model_shaderbuf,
      sizeof(model_shaderbuf) / sizeof(ShaderBuffer)
      );

  light_shaderbuf[0] =
  SHADERBUFFER_NEW(f32, state->meshes[1].vertices_len, 3, state->meshes[1].vertices, staticdraw | ShaderBuffer_Type_vertexPosition);
  light_shaderbuf[1] =
  SHADERBUFFER_NEW(f32, state->meshes[1].normals_len, 3, state->meshes[1].normals, staticdraw);
  light_shaderbuf[2] =
  SHADERBUFFER_NEW(u32, state->meshes[1].indices_len,  3, state->meshes[1].indices,  staticdraw | ShaderBuffer_Type_vertexIndex);

  state->objects[5] = RenderObject_new(
      // Shader
      get_asset(&state->resources, MySimpleShader),
      // Texture
      0, //w->render_targets->buffer[1],
      // Vertices
      light_shaderbuf,
      sizeof(light_shaderbuf) / sizeof(ShaderBuffer)
      );
}

void* mainstate_free(mainstate_state *_ /* unused */) {
	i_ctx_pop();
  return NULL;
}

StateType mainstate_update(Window *restrict w, mainstate_state *state, f64 dt) {
	StateType next_state = STATE_null;

  // Convert to seconds
  const f32 dsec = (f32)(dt / 1000000.f);

  r_clear_buffer(w->context, w->render_targets, 0);
  r_clear_buffer(w->context, w->render_targets, 1);

  // Order really shouldn't matter
  draw_model(w, 1, &state->objects[4], (vec4){CHUNK_WIDTH * WORLD_WIDTH / 2.f, CHUNK_HEIGHT / 4.f, CHUNK_LENGTH * WORLD_LENGTH / 2.f, 1});
  draw_model(w, 1, &state->objects[4], (vec4){CHUNK_WIDTH * WORLD_WIDTH / 2.f, CHUNK_HEIGHT / 4.f - 1.f, CHUNK_LENGTH * WORLD_LENGTH / 2.f + 2, 1});
  draw_model(w, 1, &state->objects[4], (vec4){CHUNK_WIDTH * WORLD_WIDTH / 2.f, CHUNK_HEIGHT / 4.f, CHUNK_LENGTH * WORLD_LENGTH / 2.f + 4, 1});
  draw_model(w, 1, &state->objects[4], (vec4){CHUNK_WIDTH * WORLD_WIDTH / 2.f + 2, CHUNK_HEIGHT / 4.f, CHUNK_LENGTH * WORLD_LENGTH / 2.f + 4, 1});
  draw_model(w, 1, &state->objects[4], (vec4){CHUNK_WIDTH * WORLD_WIDTH / 2.f, CHUNK_HEIGHT / 4.f, CHUNK_LENGTH * WORLD_LENGTH / 2.f, 1});

  draw_model(w, 1, &state->terrain.renderobj, (vec4){0, 0, 0, 1});

  // Light
  draw_model(w, 1, &state->objects[5], (vec4){7, 65, 10, 1});

  // Location should, however
  // Draw UI by their screen XY coordinates, Z is the depth/layering
  draw_model(w, 0, &state->objects[3], (vec4){0.0, 0.0, 0.0, 1});

  {
    // Anchor top left
    const vec2 padding        = {240, 240};
    const float window_width  = w->windowsize[0];
    const float window_height = w->windowsize[1];
    const float anchor_left   = -(window_width - padding[0]);
    const float anchor_top    =  (window_height - padding[1]);
    draw_model(w, 0, &state->objects[2], (vec4){anchor_left, anchor_top, 0.0, 1});
  }

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
