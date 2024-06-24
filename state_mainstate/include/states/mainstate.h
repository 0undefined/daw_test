#ifndef STATE_TITLESCREEN_H
#define STATE_TITLESCREEN_H

#include <stdbool.h>
#include <engine/core/types.h>
#include <engine/rendering/rendering.h>
#include <engine/resources.h>
#include <engine/ctrl/input.h>
//#include <engine/state.h>
//#include <engine/engine.h>
//#include <engine/ui.h>
//#include <engine/input.h>

#include <cglm/cglm.h>

typedef struct mainstate_state {
  /* Resources */
  Shader       shaders[10];
  RenderBatch batches[10];
  RenderObject objects[10];
	i_ctx input_ctx;
  binding_t input_bindings[10];
  vec3 cam_dir;
  f32 cam_dir_dt;
  vec3 cam_pos;
  vec3 cam_speed;
  vec3 cam_acc;
  Camera c;
  f64 fov;
  Resources resources;
  f64 height;
} mainstate_state;

#endif
