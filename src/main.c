#include <daw/daw.h>
#include <daw/logging.h>

#define MEMORY_SIZE 65536

Instance *p = NULL;

int main(void) {

  p = engine_init("rogue",
      420, 420,
      0,
      MEMORY_SIZE);

  //engine_fps_max(p, 10);
  engine_run(p, STATE_mainstate, NULL);
  engine_stop(p);
}
