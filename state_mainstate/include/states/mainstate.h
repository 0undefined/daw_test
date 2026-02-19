#ifndef STATE_TITLESCREEN_H
#define STATE_TITLESCREEN_H

#include <stdbool.h>
#include <daw/types.h>
#include <daw/rendering.h>
#include <daw/resources.h>
#include <daw/input.h>

#include <cglm/cglm.h>

#include <worldgen.h>

typedef struct mainstate_state {
  /* Resources */
  Shader       shaders[10];
  RenderBatch  terrain;
  RenderObject objects[10];
  u8 world[WORLD_SIZE * CHUNK_SIZE];
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
