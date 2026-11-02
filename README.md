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

The launcher is resizable and displays a black 4-by-6 grid of original app tiles, with a bottom shortcut dock. Click a tile or dock shortcut to open it; click **BACK** or press **Escape** to close. **Enter** or **Space** opens the first app. Home-screen icons enter with a stagger, while the dock and selected app animate alongside the spring transition.
