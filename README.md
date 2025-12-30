# PolarClock

A WebGL polar clock visualization built with C++ and Emscripten.

## Prerequisites

- CMake 3.16+
- Emscripten SDK (for web build)
- GLFW3 and OpenGL (for native build)

## Building for Web (Emscripten)

1. Install and activate the Emscripten SDK:
   ```bash
   source /path/to/emsdk/emsdk_env.sh
   ```

2. Configure and build:
   ```bash
   cd build-web
   emcmake cmake ..
   emmake make
   ```

3. The output files will be in `build-web/bin/`:
   - `PolarClock.html` - Main HTML file
   - `PolarClock.js` - JavaScript glue code
   - `PolarClock.wasm` - WebAssembly binary
   - `PolarClock.data` - Preloaded assets

4. Serve locally (WASM requires a web server):
   ```bash
   cd build-web/bin
   python3 -m http.server 8080
   ```
   Then open http://localhost:8080/PolarClock.html

## Building Native (Linux/macOS)

```bash
mkdir build && cd build
cmake ..
make
./bin/PolarClock
```

## Project Structure

```
├── src/           # C++ source files
├── shaders/       # GLSL shaders
├── assets/        # Fonts and other assets
├── thirdparty/    # Third-party headers (stb_truetype, etc.)
├── web/           # Emscripten shell template
└── build-web/     # Emscripten build directory
```
