# CavernBloom

CavernBloom is an original C++/OpenGL 2D platformer currently under development. This repository begins with a small graphics and application foundation written from scratch, with a single colored quad demonstrating the rendering pipeline.

**Status:** early development. Gameplay is not implemented yet.

**Stack:** C++20, CMake 3.25+, OpenGL 3.3 Core, [GLFW 3.4](https://github.com/glfw/glfw/releases/tag/3.4), [GLAD 2.0.8](https://github.com/Dav1dde/glad/releases/tag/v2.0.8), and [GLM 1.0.1](https://github.com/g-truc/glm/releases/tag/1.0.1). CMake FetchContent downloads pinned upstream tags; GLAD generates its loader in the build directory using its bundled specification.

The foundation includes a resizable 1280 x 720 window, VSync, Escape to exit, a timed input/update/render loop, file-based GLSL shaders with compile/link diagnostics, and RAII ownership of the window and GPU resources. An indexed VAO/VBO/EBO quad uses separate model and orthographic projection matrices. The centered, Y-up view stays 720 world units tall and adjusts horizontal coverage on resize, preserving proportions. The update phase is intentionally stationary.

`Game` owns the lifecycle and loop, `Renderer` owns quad geometry and transforms, and `Shader` owns the linked program. GPU resources are released before the window and GLFW.

## Build and run

On Windows, install Visual Studio 2022 or newer with **Desktop development with C++**, a Windows SDK, and CMake tools. Use a Developer PowerShell with CMake and Git on PATH. Python 3 with Jinja2 is required for GLAD; internet access is needed for the first dependency download.

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install Jinja2==3.1.6
$env:VIRTUAL_ENV = "$PWD\.venv"
$env:PATH = "$env:VIRTUAL_ENV\Scripts;$env:PATH"
cmake -S . -B build
cmake --build build --config Debug
.\build\Debug\CavernBloom.exe
```

If Visual Studio's generator is unavailable but its C++ tools are installed, set `$env:CMAKE_GENERATOR = "Ninja Multi-Config"` in the developer shell before the first configure. This foundation was built and checked on Windows with MSVC 19.51 and Ninja Multi-Config. CMake 4.3 reports upstream GLM 1.0.1 compatibility deprecation warnings; the application compiles without warnings under `/W4`.

For a single-configuration generator on Linux/macOS, configure with `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`, build with the command above, then run `./build/CavernBloom`. Install a C++20 toolchain, CMake, Git, Python/Jinja2, and platform window-system development packages required by [GLFW](https://www.glfw.org/docs/3.4/compile.html). An OpenGL 3.3-capable driver is required. These platforms are intended targets; Windows is the initial development platform.

CMake copies shaders into `build/shaders` and embeds that absolute directory in this development executable, so launching does not depend on the working directory. Rebuild after shader edits. Reconfigure/rebuild if moving the build tree; standalone distribution packaging is not implemented.

To check the foundation, launch the program, confirm a mint-colored rectangle on a dark background, resize the window and confirm the rectangle stays centered without stretching, then press Escape to close it. Build output, downloaded dependencies, and generated loader files are ignored by Git.
