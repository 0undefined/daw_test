#ifndef STATES_WORLDGEN_H
#define STATES_WORLDGEN_H
#ifdef __cplusplus
extern "C" {
#endif

#include <daw/types.h>

// first 4 bits for material type
#define BLOCK_none    0
#define BLOCK_grass   1
#define BLOCK_rock    2

// next 4 bits for shape type
#define SHAPE_filled (1 << 4)
#define SHAPE_slope  (2 << 4)

void gen_terrain(u8 *world, usize height, usize length, usize width);


#ifdef __cplusplus
}
#endif
#endif
