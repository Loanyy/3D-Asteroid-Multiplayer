# SDL2 OpenGL 3D Starter Kit

Teacher-provided C++/SDL2/OpenGL template for the course at Uppsala University.

Uses **SDL2 2.0.12** with the legacy OpenGL fixed-function pipeline (GLU) to render a 3D scene with axes, grid, and orbiting camera.

## Requirements

- **Windows 10/11** (x64)
- **Visual Studio 2019 or 2022** with the "Desktop development with C++" workload installed
- No extra downloads needed — SDL2 is included in the repo

## How to build and run

1. Clone or download this repo
2. Open `Project/Project.sln` in Visual Studio
3. Set configuration to **Debug | x64** (top toolbar)
4. Press **Ctrl+F5** (Start Without Debugging)

**Important:** Only the **Debug | x64** configuration is fully set up. Other configurations (Win32, Release) are missing SDL include/library paths and will not build without manual fixes.

## Project structure

```
Project/
├── Project.sln                  ← Open this in Visual Studio
├── Project/
│   ├── Main.cpp                 ← Entry point, SDL event loop
│   ├── Game.cpp                 ← Rendering logic (OpenGL + GLU)
│   ├── Game.h                   ← Game class header
│   ├── Project.vcxproj          ← VS build settings
│   └── Project.vcxproj.filters  ← VS filter layout
├── SDL2-2.0.12/                 ← Bundled SDL2 (headers + x64 libs)
│   ├── include/                 ← SDL2 headers
│   └── lib/x64/                 ← SDL2.lib, SDL2main.lib
└── x64/Debug/
    └── SDL2.dll                 ← Runtime DLL (must be next to .exe)
```

## SDL2 dependency note

SDL2 is bundled in the repo at `SDL2-2.0.12/` so it builds out of the box. The `.vcxproj` references it via relative paths (`$(SolutionDir)SDL2-2.0.12\...`), so **do not move or rename the SDL2 folder**.

`SDL2.dll` is placed in `x64/Debug/` because Visual Studio outputs the `.exe` there. If you switch to Release or a different output directory, you must copy `SDL2.dll` to wherever the `.exe` ends up.

## Controls

- **ESC** — Quit
- Mouse and keyboard input handlers exist in `Game.cpp` but are currently empty (ready for you to fill in)

## Toolset compatibility

The project targets **PlatformToolset v142** (VS 2019). If you open it in VS 2022, it will prompt you to retarget — accept the upgrade. This is safe and expected.

## License

SDL2 is licensed under the [zlib license](https://www.libsdl.org/license.php). Course template code is by Mikael Fridenfalk.
