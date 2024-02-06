//#include <engine/logging.h>
//#include <engine/input.h>
#include <states/mainstate.h>

void mainstate_init(mainstate_state *state) {
	INFO("Starting mainstate");
}

void mainstate_free(mainstate_state *state) {
}

StateType mainstate_update(mainstate_state *state) {
	StateType next_state = STATE_null;

	return next_state;
}
