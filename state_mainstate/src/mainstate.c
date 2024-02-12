#include <engine/engine.h>
#include <engine/core/logging.h>
#include <engine/rendering/rendering.h>
#include <states/mainstate.h>
#include <engine/core/state.h>
#include <cglm/cglm.h>

enum GameResources {
  MyVertexShader,
  MyFragmentShader,
  MyDefaultShader,
};

void mainstate_init(mainstate_state *state, void* arg) {
	INFO("Starting mainstate");

  // Use the INDICES of the assets to specify the shaders to be composed
  static u32 default_shader[] = {MyVertexShader, MyFragmentShader};

  /* 0. Declare resources */
  static asset_t mainstate_assets[] = {
    [MyVertexShader] = Declare_Shader("shader.vert"),
    [MyFragmentShader] = Declare_Shader("shader.frag"),
    [MyDefaultShader] = Declare_ShaderProgram(default_shader, sizeof(default_shader) / sizeof(default_shader[0])),
  };


  state->resources.assets = mainstate_assets;
  state->resources.assets_len = sizeof(mainstate_assets) / sizeof(mainstate_assets[0]);
  LOG("RES LEN: %lu", state->resources.assets_len);

  /* 1. Load resources */
  resources_load(&state->resources);

  // TODO: Fixup this mess below:
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


  static f32 uv[] = {
    0.0, 1.0,
    1.0, 1.0,
    1.0, 0.0,
    //0.000059f, 1.0f-0.000004f,
    //0.000103f, 1.0f-0.336048f,
    //0.335973f, 1.0f-0.335903f,

    0.0, 0.0,
    1.0, 1.0,
    1.0, 0.0,
    //1.000023f, 1.0f-0.000013f,
    //0.667979f, 1.0f-0.335851f,
    //0.999958f, 1.0f-0.336064f,

    0.667979f, 1.0f-0.335851f,
    0.336024f, 1.0f-0.671877f,
    0.667969f, 1.0f-0.671889f,

    0.0, 0.0,
    0.0, 1.0,
    1.0, 1.0,
    //1.000023f, 1.0f-0.000013f,
    //0.668104f, 1.0f-0.000013f,
    //0.667979f, 1.0f-0.335851f,

    0.0, 1.0,
    1.0, 0.0,
    0.0, 0.0,
    //0.000059f, 1.0f-0.000004f,
    //0.335973f, 1.0f-0.335903f,
    //0.336098f, 1.0f-0.000071f,

    0.0, 1.0,
    1.0, 0.0,
    0.0, 0.0,
    //0.667979f, 1.0f-0.335851f,
    //0.335973f, 1.0f-0.335903f,
    //0.336024f, 1.0f-0.671877f,

    0.0, 0.0,
    0.0, 1.0,
    1.0, 1.0,
    //1.000004f, 1.0f-0.671847f,
    //0.999958f, 1.0f-0.336064f,
    //0.667979f, 1.0f-0.335851f,

    0.0, 0.0,
    1.0, 1.0,
    1.0, 0.0,
    //0.668104f, 1.0f-0.000013f,
    //0.335973f, 1.0f-0.335903f,
    //0.667979f, 1.0f-0.335851f,

    1.0, 1.0,
    0.0, 0.0,
    0.0, 1.0,
    //0.335973f, 1.0f-0.335903f,
    //0.668104f, 1.0f-0.000013f,
    //0.336098f, 1.0f-0.000071f,

    0.000103f, 1.0f-0.336048f,
    0.000004f, 1.0f-0.671870f,
    0.336024f, 1.0f-0.671877f,

    0.000103f, 1.0f-0.336048f,
    0.336024f, 1.0f-0.671877f,
    0.335973f, 1.0f-0.335903f,

    1.0, 0.0,
    0.0, 0.0,
    1.0, 1.0,
    //0.667969f, 1.0f-0.671889f,
    //1.000004f, 1.0f-0.671847f,
    //0.667979f, 1.0f-0.335851f
  };


  //
  state->objects[0] = RenderObject_new(crate, get_asset(&state->resources, MyDefaultShader), sizeof(crate), uv, sizeof(uv));

  // Setup controls

}

void* mainstate_free(mainstate_state *state) {
  return NULL;
}

StateType mainstate_update(mainstate_state *state) {
	StateType next_state = STATE_null;

  vec3 pos = {0.f, 0.f};
  engine_draw_model(&(state->objects[0]), pos);
  vec3 pos2 = {3.5f, 0.0f, 0.5f};
  engine_draw_model(&(state->objects[0]), pos2);

	return next_state;
}
