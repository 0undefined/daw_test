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
static void gen_chunk(u32 *chunk, usize z, usize x) {
  // Flat chunks
  for (usize yy = 0; yy < CHUNK_HEIGHT / 4; yy++) {
    for (usize zz = 0; zz < CHUNK_LENGTH; zz++) {
      for (usize xx = 0; xx < CHUNK_WIDTH; xx++) {
        ivec3 pos = {xx,yy,zz};
        chunk[global_idx(pos)] = BLOCK_grass;
        //chunk[yy * CHUNK_LENGTH * CHUNK_WIDTH
        //  + zz * CHUNK_WIDTH
        //  + xx] = BLOCK_grass;
      }
    }
  }
    for (usize zz = 0; zz < CHUNK_LENGTH; zz++) {
      for (usize xx = 0; xx < CHUNK_WIDTH; xx++) {
        if (zz % 2 != 0) continue;
        if (xx % 2 != 0) continue;
        chunk[(CHUNK_HEIGHT / 4 + 0) * CHUNK_LENGTH * CHUNK_WIDTH
          + zz * CHUNK_WIDTH
          + xx] = BLOCK_rock;
      }
    }
}

void gen_terrain(u32 *world) {
  if (world == NULL) {
    world = calloc(WORLD_SIZE * CHUNK_SIZE, sizeof(u8));
  }

  for (usize z = 0; z < WORLD_LENGTH; z++) {
    for (usize x = 0; x < WORLD_WIDTH; x++) {
      gen_chunk(&world[
          z * WORLD_WIDTH * CHUNK_SIZE
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


  // Mask everything
  for (usize z = 0; z < WORLD_LENGTH; z++) {
    for (usize x = 0; x < WORLD_WIDTH; x++) {

      u32 * restrict chunk = &world[
          z * WORLD_WIDTH * CHUNK_SIZE
          + x * CHUNK_SIZE
      ];

      for (isize yy = 0; yy < CHUNK_HEIGHT; yy++) {
        for (isize zz = 0; zz < CHUNK_LENGTH; zz++) {
          for (isize xx = 0; xx < CHUNK_WIDTH; xx++) {

            u32 mask = 0;

            // TODO: Fix OOB access
            for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
              if (i == 0 && yy - 1 < 0) continue;
              if (j == 0 && zz - 1 < 0) continue;
              if (k == 0 && xx - 1 < 0) continue;
              if (i == 2 && yy + 1 >= CHUNK_HEIGHT) continue;
              if (j == 2 && zz + 1 >= CHUNK_LENGTH) continue;
              if (k == 2 && xx + 1 >= CHUNK_WIDTH) continue;

              if (
                  chunk[(i + yy - 1) * CHUNK_LENGTH * CHUNK_WIDTH
                  + (j + zz - 1) * CHUNK_WIDTH
                  + (k + xx - 1)] & ((1 << 5) - 1)
                 ) {
                mask |= 1 << ((i * 9) + j * 3 + k);
              }
            }
            }
            }

            if (yy == 0) {
              mask |= FILLED_MASK_BOT;
            }
            chunk[yy * CHUNK_LENGTH * CHUNK_WIDTH
              + zz * CHUNK_WIDTH
              + xx] |= mask << 5;
          }
        }
      }

    }
  }

  // Mask chunk borders
  for (usize z = 0; z < WORLD_LENGTH; z++) {
    for (usize x = 0; x < WORLD_WIDTH; x++) {
      u32 * restrict chunk = &world[
          z * WORLD_WIDTH * CHUNK_SIZE
          + x * CHUNK_SIZE
      ];

      // LEFT (-X)
      if (x > 0) {
        u32 * restrict lchunk = &world[
            z * WORLD_WIDTH * CHUNK_SIZE
            + (x - 1) * CHUNK_SIZE
        ];
        // local YZ
        for (isize ly = 0; ly < CHUNK_HEIGHT; ly++) {
          for (isize lz = 0; lz < CHUNK_LENGTH; lz++) {
            u32 mask = 0;

            // neighbouring xy
            for (isize i = 0; i < 3; i++) {
              for (isize j = 0; j < 3; j++) {
                if (i == 0 && ly - 1 < 0) continue;
                if (j == 0 && lz - 1 < 0) continue;
                if (i == 2 && ly + 1 >= CHUNK_HEIGHT) continue;
                if (j == 2 && lz + 1 >= CHUNK_LENGTH) continue;

                ivec3 pos = {CHUNK_WIDTH - 1, i + ly - 1, j + lz - 1};
                if (lchunk[global_idx(pos)] & (((1 << 5) - 1))) {
                  mask |= 1 << (i * 9 + j * 3);
                }
              }
            }
            ivec3 pos = {0, ly, lz};
            chunk[ly * CHUNK_LENGTH * CHUNK_WIDTH
              + lz * CHUNK_WIDTH
              + 0] |= mask << 5;
          }
        }
      }
      // RIGHT (+X)
      if (x < WORLD_WIDTH - 1) {
        u32 * restrict rchunk = &world[
            z * WORLD_WIDTH * CHUNK_SIZE
            + (x + 1) * CHUNK_SIZE
        ];

        // local YZ
        for (isize ly = 0; ly < CHUNK_HEIGHT; ly++) {
          for (isize lz = 0; lz < CHUNK_LENGTH; lz++) {
            u32 mask = 0;

            // neighbouring xy
            for (isize i = 0; i < 3; i++) {
              for (isize j = 0; j < 3; j++) {
                if (i == 0 && ly - 1 < 0) continue;
                if (j == 0 && lz - 1 < 0) continue;
                if (i == 2 && ly + 1 >= CHUNK_HEIGHT) continue;
                if (j == 2 && lz + 1 >= CHUNK_LENGTH) continue;

                ivec3 pos = {0, i + ly - 1, j + lz - 1};
                if (rchunk[global_idx(pos)] & (((1 << 5) - 1))) {
                  mask |= 1 << (i * 9 + j * 3 + 2);
                }
              }
            }
            ivec3 pos = {CHUNK_WIDTH - 1, ly, lz};
            chunk[ly * CHUNK_LENGTH * CHUNK_WIDTH
              + lz * CHUNK_WIDTH
              + (CHUNK_WIDTH - 1)] |= mask << 5;
          }
        }
      }
      // BACK (-Z)
      if (z > 0) {
        u32 * restrict bchunk = &world[
            (z - 1) * WORLD_WIDTH * CHUNK_SIZE
            + x * CHUNK_SIZE
        ];
        // local YZ
        for (isize ly = 0; ly < CHUNK_HEIGHT; ly++) {
          for (isize lx = 0; lx < CHUNK_LENGTH; lx++) {
            u32 mask = 0;

            // neighbouring xy
            for (isize i = 0; i < 3; i++) {
              for (isize j = 0; j < 3; j++) {
                if (i == 0 && ly - 1 < 0) continue;
                if (j == 0 && lx - 1 < 0) continue;
                if (i == 2 && ly + 1 >= CHUNK_HEIGHT) continue;
                if (j == 2 && lx + 1 >= CHUNK_WIDTH) continue;

                ivec3 pos = {j + lx - 1, i + ly - 1, CHUNK_LENGTH - 1};
                if (bchunk[global_idx(pos)] & (((1 << 5) - 1))) {
                  mask |= 1 << (i * 9 + j);
                }
              }
            }
            ivec3 pos = {lx, ly, 0};
            chunk[ly * CHUNK_LENGTH * CHUNK_WIDTH
              + 0
              + lx] |= mask << 5;
          }
        }
      }
      // FRONT (+Z)
      if (z < WORLD_LENGTH - 1) {
        u32 * restrict rchunk = &world[
            (z + 1) * WORLD_WIDTH * CHUNK_SIZE
            + x * CHUNK_SIZE
        ];

        // local YZ
        for (isize ly = 0; ly < CHUNK_HEIGHT; ly++) {
          for (isize lx = 0; lx < CHUNK_WIDTH; lx++) {
            u32 mask = 0;

            // neighbouring xy
            for (isize i = 0; i < 3; i++) {
              for (isize j = 0; j < 3; j++) {
                if (i == 0 && ly - 1 < 0) continue;
                if (j == 0 && lx - 1 < 0) continue;
                if (i == 2 && ly + 1 >= CHUNK_HEIGHT) continue;
                if (j == 2 && lx + 1 >= CHUNK_WIDTH) continue;

                ivec3 pos = {j + lx - 1, i + ly - 1, 0};
                if (rchunk[global_idx(pos)] & (((1 << 5) - 1))) {
                  mask |= 1 << (i * 9 + j + 6);
                }
              }
            }
            ivec3 pos = {lx, ly, CHUNK_LENGTH - 1};
            chunk[ly * CHUNK_LENGTH * CHUNK_WIDTH
              + (CHUNK_LENGTH - 1) * CHUNK_WIDTH
              + lx] |= mask << 5;
          }
        }
      }


    }
  }
}
