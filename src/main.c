#include <engine/engine.h>
#include <rogue/resources.h>

#define MEMORY_SIZE 65536

int main(void) {

	Platform *p = engine_init("rogue",
      0,0,
	                          1.f, 0,
	                          MEMORY_SIZE,
	                          NULL,
	                          NULL);

	engine_run(p, STATE_titlescreen);
  engine_stop(p);
}
