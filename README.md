# Engine testing repository

This repo serves as testing grounds for feature development of the daw engine.

Building requirenments are the same as the engine itself, ie. assimp, glad,
glfw, and stb.

I strive to keep this branch as compatible as possible with the latest version
of the [engine](https://codeberg.org/onelin/daw).

building currently requires building as shared library / debug build:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

You can replace "Debug" with "Release" for a release version.

Because the debug build uses hot reloading, we need to set `LD_LIBRARY_PATH` to
our build folder before running:
```
LD_LIBRARY_PATH=build ./build/rogue
```
