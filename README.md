# RecreatePhoneUi
Its what the name suggests, please credit me if you use this

## C++ launcher

The OpenGL 4.3 Core launcher uses the bundled GLAD and GLM headers, GLFW, and FreeType.

Install the system dependencies on Ubuntu:

```bash
sudo apt install build-essential cmake libglfw3-dev libfreetype-dev
```

Build and run:

```bash
cmake -S . -B build
cmake --build build
./build/field_kit
```

### Windows 10

Install Visual Studio 2019 or newer with the Desktop development with C++ workload and CMake. No vcpkg is needed; CMake downloads GLFW and FreeType automatically if they are not already installed. From Command Prompt (`cmd`), run:

```bat
cmake -S . -B build -A x64 -DFIELDKIT_FETCH_DEPENDENCIES=ON
cmake --build build --config Release
```

Aggressive Release optimizations are enabled by default. On MSVC this includes `/O2`, link-time code generation, and linker dead-code removal. GCC/Clang builds use `-O3` and tune x86 builds for Silvermont, the instruction-set baseline supported by the Celeron N3050; AVX is not enabled. Turn the profile off with `-DFIELDKIT_AGGRESSIVE_OPTIMIZATIONS=OFF` if you need a more portable binary. The N3050's graphics driver must also support OpenGL 4.3 for the launcher to start.

The launcher is resizable and displays a black 4-by-6 grid of original app tiles, with a bottom shortcut dock. Click a tile or dock shortcut to open it; click **BACK** or press **Escape** to close. **Enter** or **Space** opens the first app. Home-screen icons enter with a stagger, while the dock and selected app animate alongside the spring transition.
