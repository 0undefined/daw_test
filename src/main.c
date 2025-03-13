#include <engine/engine.h>
#include <engine/core/logging.h>

#define MEMORY_SIZE 65536

int main(void) {

  Platform *p = engine_init("rogue",
      420, 420,
      1.f, 0,
      MEMORY_SIZE);

  //engine_fps_max(p, 300);
  engine_run(p, STATE_mainstate, NULL);
  engine_stop(p);
}
