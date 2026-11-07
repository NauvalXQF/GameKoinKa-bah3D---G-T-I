# Tawaf 3D — Ka'bah Coin Game

A 3D game built with **C++ and OpenGL/GLUT**: walk around the Ka'bah, collect coins, avoid guards, and beat the timer.

## Features

- First-person and third-person camera (TPS/FPS toggle)
- Mouse look with pointer capture
- Jumping with gravity, animated walk cycle
- Collectible coins, roaming guards, and a 5-minute countdown
- HUD with score, timer, and camera info
- Textured environment (Ka'bah, walls) and generated sound effects

## Controls

| Input | Action |
|---|---|
| `W A S D` / arrow keys | Move / turn |
| `Space` | Jump |
| Left click | Capture mouse (look around) |
| Right click | Release mouse |
| `V` (in-game) | Toggle TPS/FPS camera |

## Build (Windows + MinGW + freeglut)

```bash
mingw32-make        # or: make
Tawaf.exe
```

Required libraries: `-lopengl32 -lglu32 -lfreeglut -lm -lwinmm`.

Keep the `.bmp` textures and `.wav` sounds in the same directory as the executable.

## Structure

```
main.cpp            window, input, HUD, game loop
environment.cpp/h   Ka'bah, walls, coins, guards
player.cpp/h        player state and rendering
globals.h           shared constants
Makefile            MinGW build
```

## Notes

This is an academic graphics project. Prebuilt binaries and object files are kept in the repository for convenience; the recommended workflow is to rebuild from source.
