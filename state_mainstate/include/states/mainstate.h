#ifndef STATE_TITLESCREEN_H
#define STATE_TITLESCREEN_H

#include <stdbool.h>
#include <engine/core/types.h>
#include <engine/rendering/rendering.h>
#include <engine/resources.h>
//#include <engine/state.h>
//#include <engine/engine.h>
//#include <engine/ui.h>
//#include <engine/input.h>

typedef struct mainstate_state {
  /* Resources */
  Shader       shaders[10];
  RenderObject objects[10];
  Resources resources;
  f64 height;
} mainstate_state;

#endif
