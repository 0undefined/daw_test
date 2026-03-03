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
#define WORLD_WIDTH  16
#define WORLD_LENGTH 16
#define WORLD_SIZE   (WORLD_LENGTH * WORLD_WIDTH)


// first 5 bits for material type
#define BLOCK_none    0
#define BLOCK_dirt    1
#define BLOCK_grass   2
#define BLOCK_rock    3
#define BLOCK_ore     4
// ... Room for 32 in total

// next 4 bits for shape type
//#define SHAPE_none        (0 << 4)
//#define SHAPE_slope_north (1 << 4)
//#define SHAPE_slope_east  (2 << 4)
//#define SHAPE_slope_south (3 << 4)
//#define SHAPE_slope_west  (4 << 4)
// ... Room for 16 in total


// 24 bits for storing neighbouring blocks
// Used for kernel operations

#define FILLED_TOP_NW        (1 <<  5)
#define FILLED_TOP_N         (1 <<  6)
#define FILLED_TOP_NE        (1 <<  7)
#define FILLED_TOP_W         (1 <<  8)
#define FILLED_TOP_C         (1 <<  8)
#define FILLED_TOP_E         (1 <<  9)
#define FILLED_TOP_SW        (1 << 10)
#define FILLED_TOP_S         (1 << 11)
#define FILLED_TOP_SE        (1 << 12)

#define FILLED_NW            (1 << 13)
#define FILLED_N             (1 << 14)
#define FILLED_NE            (1 << 15)
#define FILLED_W             (1 << 16)
#define FILLED_C             (1 << 16)
#define FILLED_E             (1 << 17)
#define FILLED_SW            (1 << 18)
#define FILLED_S             (1 << 19)
#define FILLED_SE            (1 << 20)

#define FILLED_BOT_NW        (1 << 21)
#define FILLED_BOT_N         (1 << 22)
#define FILLED_BOT_NE        (1 << 23)
#define FILLED_BOT_W         (1 << 24)
#define FILLED_BOT_C         (1 << 24)
#define FILLED_BOT_E         (1 << 25)
#define FILLED_BOT_SW        (1 << 26)
#define FILLED_BOT_S         (1 << 27)
#define FILLED_BOT_SE        (1 << 28)

// 0b11111 aka. 31 or 2**5
#define FILLED_MASK_TOP (u32)(\
  FILLED_TOP_NW | FILLED_TOP_N | FILLED_TOP_NE | \
  FILLED_TOP_W  | FILLED_TOP_C | FILLED_TOP_E  | \
  FILLED_TOP_SW | FILLED_TOP_S | FILLED_TOP_SE \
)
#define FILLED_MASK (u32)(\
  FILLED_NW | FILLED_N | FILLED_NE | \
  FILLED_W  | FILLED_C | FILLED_E  | \
  FILLED_SW | FILLED_S | FILLED_SE \
)
#define FILLED_MASK_BOT (u32)(\
  FILLED_BOT_NW | FILLED_BOT_N | FILLED_BOT_NE | \
  FILLED_BOT_W  | FILLED_BOT_C | FILLED_BOT_E  | \
  FILLED_BOT_SW | FILLED_BOT_S | FILLED_BOT_SE \
)

#define FILLED_MASK_ALL (u32)(~((1 << 5) - 1))

#define SURROUNDED(x) ((x & FILLED_MASK_ALL) == FILLED_MASK_ALL)

void gen_terrain(u32 *world);


#ifdef __cplusplus
}
#endif
#endif
