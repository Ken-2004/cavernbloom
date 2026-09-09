# CavernBloom

CavernBloom is an original C++/OpenGL 2D platformer currently under development. Its application, rendering, and movement foundation is written from scratch, using colored rectangles for a placeholder player and temporary flat floor.

**Status:** early development. Player movement is implemented; levels and a general platform collision system are not implemented yet.

**Stack:** C++20, CMake 3.25+, OpenGL 3.3 Core, [GLFW 3.4](https://github.com/glfw/glfw/releases/tag/3.4), [GLAD 2.0.8](https://github.com/Dav1dde/glad/releases/tag/v2.0.8), and [GLM 1.0.1](https://github.com/g-truc/glm/releases/tag/1.0.1). CMake FetchContent downloads pinned upstream tags; GLAD generates its loader in the build directory using its bundled specification.

The foundation includes a resizable 1280 x 720 window, VSync, fixed-timestep simulation at 120 Hz, file-based GLSL shaders with compile/link diagnostics, and RAII ownership of the window and GPU resources. Reusable colored rectangles share one indexed VAO/VBO/EBO quad with separate model and orthographic projection matrices. Positions denote rectangle centers, +X points right, and +Y points up. The view stays 720 world units tall and adjusts horizontal coverage on resize without changing physics state.

`Game` owns the lifecycle, input, and accumulator loop; `Player` owns movement and gravity without OpenGL resources; `Renderer` owns shared quad geometry; and `Shader` owns the linked program. GPU resources are released before the window and GLFW. Simulation constants live in `include/SimulationConfig.hpp`: movement 280 units/s, gravity -1800 units/s², jump impulse 650 units/s, and frame-delta clamp 0.25 s. Excess elapsed time after a long stall is discarded to bound catch-up work; rendering uses the latest simulation state without interpolation.

Move with **A/D** or **Left/Right**, jump with **Space**, and exit with **Escape**. Opposite directions cancel and releasing movement stops immediately on the next simulation step. A fresh Space press jumps only while grounded; holding it does not auto-jump on landing. The 40 x 64 player starts above a temporary infinite Y boundary at -240, drawn as a floor across the view. There are no side walls or camera tracking, so the player can move beyond the visible area.

## Build and run

On Windows, install Visual Studio 2022 or newer with **Desktop development with C++**, a Windows SDK, and CMake tools. Use a Developer PowerShell with CMake and Git on PATH. Python 3 with Jinja2 is required for GLAD; internet access is needed for the first dependency download.

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install Jinja2==3.1.6
$env:VIRTUAL_ENV = "$PWD\.venv"
$env:PATH = "$env:VIRTUAL_ENV\Scripts;$env:PATH"
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
.\build\Debug\CavernBloom.exe
```

If Visual Studio's generator is unavailable but its C++ tools are installed, set `$env:CMAKE_GENERATOR = "Ninja Multi-Config"` in the developer shell before the first configure. This foundation was built and checked on Windows with MSVC 19.51 and Ninja Multi-Config. CMake 4.3 reports upstream GLM 1.0.1 compatibility deprecation warnings; the application compiles without warnings under `/W4`.

For a single-configuration generator on Linux/macOS, configure with `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`, build with the command above, then run `./build/CavernBloom`. Install a C++20 toolchain, CMake, Git, Python/Jinja2, and platform window-system development packages required by [GLFW](https://www.glfw.org/docs/3.4/compile.html). An OpenGL 3.3-capable driver is required. These platforms are intended targets; Windows is the initial development platform.

CMake copies shaders into `build/shaders` and embeds that absolute directory in this development executable, so launching does not depend on the working directory. Rebuild after shader edits. Reconfigure/rebuild if moving the build tree; standalone distribution packaging is not implemented.

To check the foundation, let the mint-colored player fall onto the slate-colored floor, try movement and jumping, hold opposite directions, and keep Space held through a landing. Resize the window to check proportions, then press Escape to close it. Build output, downloaded dependencies, and generated loader files are ignored by Git.

The CTest physics check covers falling, landing, movement, jumping, airborne jump rejection, and fixed steps grouped into 30/60/144/240 FPS schedules without requiring a window. Configure with `-DBUILD_TESTING=OFF` to omit it.
