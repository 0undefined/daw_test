# Engine testing repository

This repo serves as testing grounds for feature development of the daw engine.

Building requirenments are the same as the engine itself, ie. assimp, glad,
glfw, and stb.

building currently requires building as shared library / debug build:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

running:
```
LD_LIBRARY_PATH=build ./build/rogue
```
