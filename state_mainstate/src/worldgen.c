#include <stdlib.h>

#include <worldgen.h>

void gen_terrain(u8 *world, usize height, usize length, usize width) {
  if (world == NULL) {
    world = calloc(width * length * height, sizeof(u8));
  }

  // y: height
  // z: depth/length
  // x: width

  // This is pretty cache unfriendly
  for (usize y = 0; y < height / 2; y++) {
    for (usize z = 0; z < length; z++) {
      for (usize x = 0; x < width; x++) {
        world[y * width * length
          + z * width
          + x] = BLOCK_grass;
      }
    }
  }

  for (usize y = 0; y < 4; y++) {
    for (usize z = 0; z < 8; z++) {
      for (usize x = 0; x < 8; x++) {
        if (z > 0 && z < 7 && x > 0 && x < 7) continue;
        world[((height / 2 + 1) + y) * width * length
          + (z + (length / 2 - 4)) * width
          + (x + (width / 2 - 4))] = BLOCK_rock;
      }
    }
  }

  // Doorway
  world[(height / 2 + 1) * width * length
    + (length / 2) * width
    + (width / 2 - 4)] = BLOCK_none;
  world[((height / 2 + 1) + 1) * width * length
    + (length / 2) * width
    + (width / 2 - 4)] = BLOCK_none;
}
