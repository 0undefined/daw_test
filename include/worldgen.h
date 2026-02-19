#ifndef STATES_WORLDGEN_H
#define STATES_WORLDGEN_H
#ifdef __cplusplus
extern "C" {
#endif

#include <daw/types.h>

// amounts to 32768 blocks per chunk
#define CHUNK_WIDTH  16
#define CHUNK_LENGTH 16
#define CHUNK_HEIGHT 128
#define CHUNK_SIZE   (CHUNK_WIDTH * CHUNK_LENGTH * CHUNK_HEIGHT)



// World size is defined in terms of number of chunks. Chunks are not stacked
// vertically
#define WORLD_WIDTH  4
#define WORLD_LENGTH 4
#define WORLD_SIZE   (WORLD_LENGTH * WORLD_WIDTH)


// first 4 bits for material type, 16 types in total
#define BLOCK_none    0
#define BLOCK_grass   1
#define BLOCK_rock    2

// next 4 bits for shape type
#define SHAPE_none        (0 << 4)
#define SHAPE_slope_north (1 << 4)
#define SHAPE_slope_east  (2 << 4)
#define SHAPE_slope_south (3 << 4)
#define SHAPE_slope_west  (4 << 4)

void gen_terrain(u8 *world);


#ifdef __cplusplus
}
#endif
#endif
