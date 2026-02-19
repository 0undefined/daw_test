#include <stdlib.h>
#include <cglm/ivec3.h>

#include <worldgen.h>

// Store the xyz position given an index
static void global_position(ivec3 *pos, usize idx) {
  const isize chunk_idx = idx / CHUNK_SIZE;
  const isize local_idx = idx % CHUNK_SIZE;
  const isize local_x = local_idx % CHUNK_WIDTH;
  const isize local_y = local_idx / (CHUNK_LENGTH * CHUNK_WIDTH);
  const isize local_z = (local_idx - (CHUNK_LENGTH * CHUNK_WIDTH * local_y)) / CHUNK_WIDTH;

  const isize chunk_x = chunk_idx % WORLD_WIDTH;
  const isize chunk_z = chunk_idx / WORLD_WIDTH;

  const isize y = local_y; // height
  const isize z = local_z + chunk_z * CHUNK_LENGTH; // length
  const isize x = local_x + chunk_x * CHUNK_WIDTH; // width

  *pos[0] = x;
  *pos[1] = y;
  *pos[2] = z;
}

// Return the index from xyz position
static usize global_idx(ivec3 pos) {
  int x = pos[0];
  int y = pos[1];
  int z = pos[2];

  usize chunk_x = x / CHUNK_WIDTH;
  usize chunk_z = z / CHUNK_LENGTH;
  usize chunk_idx = chunk_z * WORLD_WIDTH + chunk_x;

  usize local_x = x % CHUNK_WIDTH;
  usize local_y = y;
  usize local_z = z % CHUNK_LENGTH;
  usize local_idx = local_y * CHUNK_LENGTH * CHUNK_WIDTH
                  + local_z * CHUNK_WIDTH
                  + local_x;

  return chunk_idx * CHUNK_SIZE + local_idx;
}

// Chunks are laid out in worldspace as [c1, ..., cN]
static void gen_chunk(u8 *chunk, usize z, usize x) {
  // Flat chunks
  for (usize yy = 0; yy < CHUNK_HEIGHT / 4; yy++) {
    for (usize zz = 0; zz < CHUNK_LENGTH; zz++) {
      for (usize xx = 0; xx < CHUNK_WIDTH; xx++) {
        chunk[yy * CHUNK_LENGTH * CHUNK_WIDTH
          + zz * CHUNK_WIDTH
          + xx] = BLOCK_grass;
      }
    }
  }
}

void gen_terrain(u8 *world) {
  if (world == NULL) {
    world = calloc(WORLD_SIZE * CHUNK_SIZE, sizeof(u8));
  }

  for (usize z = 0; z < WORLD_WIDTH; z++) {
    for (usize x = 0; x < WORLD_LENGTH; x++) {
      gen_chunk(&world[
          z * WORLD_LENGTH * CHUNK_SIZE
          + x * CHUNK_SIZE
      ], z, x);
    }
  }

  // y: height
  // z: depth/length
  // x: width

  // "house"
  for (usize y = 0; y < 4; y++) {
    for (usize z = 0; z < 7; z++) {
      for (usize x = 0; x < 9; x++) {
        if (z > 0 && z < 6 && x > 0 && x < 8) continue;
        ivec3 pos = { CHUNK_WIDTH * WORLD_WIDTH / 2 + x - 4
                    , ((CHUNK_HEIGHT / 4) + y)
                    , CHUNK_LENGTH * WORLD_LENGTH / 2 + z - 4
        };
        world[global_idx(pos)] = BLOCK_rock;
      }
    }
  }

  // Doorway
  ivec3 pos = { CHUNK_WIDTH * WORLD_WIDTH / 2
              , (CHUNK_HEIGHT / 4)
              , CHUNK_LENGTH * WORLD_LENGTH / 2 + 2
  };
  world[global_idx(pos)] = BLOCK_none;
  pos[1]++;
  world[global_idx(pos)] = BLOCK_none;
}
