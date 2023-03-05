# dungeons of ufeff

This is a rebuild of a git repository from ufeffjam 1, a private game jam which
ran for approximately a month some time around July to August 2020. The tag
v1.0 represents the source code as the game was released for the game jam,
though it probably won't compile without the correct dependencies. In
particular, MSVC has moved headers around since then.

# Building

## Requirements

The master branch is currently guaranteed to build with the following:

- MSVC (cl 19.35.32215) *or* clang (15.0.7)
- Python 3.11
- Tiled 1.9.2
- Blender 2.93.4
- cmake 3.25.2

All of these programs must be in `PATH`.

## Build commands

```sh
git clone git@github.com:jcmoyer/dungeons_of_ufeff.git
cd dungeons_of_ufeff
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=dist ..
cmake --build .
cmake --install .
```

The cmake script will build models and maps and copy all the final assets to
their correct locations under `dungeons_of_ufeff/build/dist`.

# License

Third party license information for libraries found under `thirdparty/` is
located in `license.txt`.

All code under `src/` and `scripts/` explicitly has no license.

Asset attribution can be found in `attribution.txt`. Any files not listed there
such as the models and music are my own work.
