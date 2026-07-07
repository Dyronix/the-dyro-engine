# DyxEngine

A small DirectX 12 engine for 2D games, written to be read. Every system is
one small class with one job, so you can dig into any part of the engine and
understand it completely.

## Documentation

Open [docs/html/index.html](docs/html/index.html) in a browser for the full
searchable API reference plus guide pages with examples for every system.
(Maintainers: regenerate it with `docs/generate_docs.bat` after changing the
public headers or the guide pages.)

## Getting started

You need **Visual Studio 2026** (or **2022**) with the *Desktop development
with C++* workload and a recent **Windows 10/11 SDK** (both are part of the
default workload installation).

```
generate.bat                 # generates build/dyx_engine.sln (VS2026 by default)
build.bat -debug             # or open the solution and press F5
```

`generate.bat` targets Visual Studio 2026 by default; pass `-2022` to target
Visual Studio 2022 instead. Switching between them re-generates from scratch.
(VS2026 generation requires CMake 4.2 or newer.)

The solution sets `dyx_game` as the startup project. Run it and you should
see a checkerboard, a rotating red quad, a bouncing ball and a small hud.
Move the ball with wasd, tint it with space, resize it with the mouse wheel.

## Making your own game

Look at `source/games/dyx_game`: a game is a class that derives from
`dyx::game` and overrides four functions. To add code, just create new
`.cpp`/`.h` files in `source/games/dyx_game/private` and build; they are
picked up automatically, no build-system editing needed. Want to add a
second game alongside the demo instead of replacing it? See "Adding a second
game" in [the getting started guide](docs/html/page_getting_started.html).

```cpp
class my_game : public dyx::game
{
public:
    void initialize(dyx::engine& engine) override;  // load your textures here
    void update(float delta_seconds) override;       // game logic here
    void draw(dyx::renderer_2d& renderer) override; // draw sprites here
    void shutdown() override;                        // cleanup here
};
```

Drawing is done with a single call:

```cpp
renderer.draw_sprite(*my_texture, position, size, rotation, tint);
```

`position` is the sprite center in pixels, with (0, 0) in the top-left corner
of the window. Sprites are drawn in the order you submit them, so draw the
background first. The renderer also draws parts of a texture (sprite sheets),
rectangles, lines and text:

```cpp
renderer.draw_sprite(*sheet, source_rect, position, size);  // one frame of a sheet
renderer.draw_rect(area, color);                            // filled rectangle
renderer.draw_rect_outline(area, thickness, color);
renderer.draw_line(from, to, thickness, color);
renderer.draw_text(font, "hello", position, pixel_height);  // bitmap font text
```

Keyboard and mouse come from `engine.get_input()`:

```cpp
if (input.is_key_down(dyx::key::a))         { /* every frame while held */ }
if (input.was_key_pressed(dyx::key::space)) { /* only the frame it went down */ }
dyx::vec2 mouse = input.get_mouse_position();
```

Math is [glm](https://github.com/g-truc/glm) under dyx names: `dyx::vec2`,
`dyx::mat4`, `dyx::lerp`, `dyx::clamp`, ... (see `core/math.h`), plus a 2D
`dyx::rect` for bounds and overlap checks (`core/rect.h`). Utilities:
`dyx::timer` (measure time), `dyx::random_float` and friends
(`core/random.h`), smooth `dyx::noise_2d` (`core/noise.h`), logging via
`dyx::log::info/warn/error` and asserts via `DYX_ASSERT` /
`dyx::fatal_error` (`core/assert.h`).

## Folder structure

```
cmake/            build-system helper scripts (each one is commented)
content/          textures, fonts and other assets (copied next to the exe at build time)
shaders/          hlsl shader source files (compiled at build time)
source/
  engine/         the engine static library (namespace dyx)
    public/       headers your game may include
    private/      implementation details
  games/          one folder per game (see source/games/CMakeLists.txt)
    dyx_game/    the demo game; replace it, or add more games next to it
                  (new .cpp/.h files in its private/ folder are picked up automatically)
  tools/
    shader_compiler/  build tool that compiles hlsl to directx bytecode
  third_party/    code you use but do not need to read:
                  stb (image decoding, perlin noise), glm (math),
                  rnd (random number internals)
```

Everything inside `source/engine` and `source/game` is written to be
understood. When code is complex but not interesting to read (decoding a png,
the bit-twiddling inside a random generator) it lives in `source/third_party`
and the engine wraps it behind a small, readable api.

## How the engine works

`dyx::engine::run()` brings the systems up in dependency order, then runs the
main loop. Each system is one class:

| class | job |
|---|---|
| `window` | the win32 window the engine renders into |
| `input` | keyboard and mouse state (down / pressed / released) |
| `device` | picks a graphics card and creates the directx 12 device |
| `adapter_selection` | enumerates and scores all graphics cards |
| `shader_model` | queries which shader model the card supports |
| `command_queue` | submits work to the gpu and synchronizes cpu/gpu |
| `swap_chain` | owns the back buffers that end up on screen |
| `descriptor_heap` | hands out descriptor slots (texture views) |
| `shader_library` | loads compiled shaders (*.cso) from disk |
| `pso_cache` | saves/loads pipeline state objects to/from disk |
| `texture_loader` | decodes image files / raw pixels and uploads them to the gpu |
| `font` | describes a fixed-grid bitmap font atlas for draw_text |
| `renderer_2d` | draws textured quads (this is a 2D engine: everything is a quad) |

### Graphics card selection

Like Unreal, the engine looks at every graphics card in the machine before
picking one. Each adapter gets a score based on its dedicated video memory
(the dominant factor), maximum feature level and highest shader model.
Software adapters (WARP) score zero and are only used when no real card
exists. Set `engine_settings::gpu_preference` to
`adapter_preference::lowest_score` to run on the weakest card, handy for
testing how your game behaves on low-end hardware. The scoring of every card
is printed to the console at startup.

### Shader compilation (the toolchain)

DirectX 12 can only load *compiled* shaders. The `shader_compiler` tool (built
from `source/tools/shader_compiler`, using Microsoft's DXC compiler) turns
`.hlsl` files into `.cso` bytecode.

It is wired into the build by `DYX_COMPILE_SHADERS` in
`cmake/compile_shaders.cmake`: every `*.hlsl` file in `/shaders` is compiled into
`<build>/<config>/content/shaders/` as part of a normal build. CMake tracks
the dependencies, so a shader is only recompiled when its source changed or
when it has never been compiled, just like C++ files.

The shader profile is derived from the file name: `foo_vs.hlsl` becomes a
vertex shader, `foo_ps.hlsl` a pixel shader. To add a shader, drop the file in
`/shaders` and build.

### PSO caching

Creating a pipeline state object makes the gpu driver compile shader bytecode
into real gpu instructions, which is slow. The `pso_cache` wraps directx's
`ID3D12PipelineLibrary`: pipelines created during a run are serialized to
`cache/pso_cache.bin` on shutdown and loaded instantly on the next start.
The cache invalidates itself when the driver, gpu or a shader changed. If
DirectX rejects the file on disk, the engine logs a warning, deletes that
unusable cache and rebuilds pipelines from scratch; the fresh cache is saved
again on shutdown. Watch the console: `created from scratch` on the first run,
`loaded from the pso cache` after.

### Textures

`texture_loader` uses stb_image, so png/jpg/bmp/tga all work. The gpu cannot
read cpu memory directly: pixels are written into an *upload buffer*, the gpu
copies them into the final texture resource, and a *shader resource view* is
created so shaders can sample it. All of that lives in one readable function:
`texture_loader::create_from_pixels`, which you can also call yourself with
raw rgba pixels to build textures procedurally (the demo generates a noise
texture and a sprite sheet this way).

Put your images in `content/textures`; the build copies the whole `content`
folder next to the executable.

### Text

`draw_text` renders with a bitmap font: one atlas texture holding every
character in a fixed grid (`content/fonts/font_8x8.png`, based on the public
domain [font8x8](https://github.com/dhepper/font8x8) by Daniel Hepper). The
`font` struct describes the grid; each character becomes one quad that shows
its part of the atlas. Text rendering is just sprite sheet drawing.

### The frame

`renderer_2d` owns the frame loop plumbing. One unit quad lives in gpu
memory; every `draw_sprite` call draws that same quad with its own
transform, texture and tint (passed as root constants + a descriptor table).
The engine keeps one command allocator per back buffer and uses a fence to
wait until the gpu released a buffer before recording into it again. This is
the classic "frames in flight" pattern, in its smallest possible form.

## Ideas to extend it

- sound
- sprite batching (every draw call currently draws one quad)
- a texture cache so loading the same file twice reuses the gpu texture
- gamepad support in `input`
- a fixed timestep update loop
