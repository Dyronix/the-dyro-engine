# Under the hood {#page_under_the_hood}

You do not need anything on this page to make a game — but the whole point
of this engine is that you *can* read it. dyro::engine::run brings the
systems up in dependency order, then runs the main loop. Each system is one
small class:

| class | job |
|---|---|
| dyro::window | the win32 window the engine renders into |
| dyro::input | keyboard and mouse state (down / pressed / released) |
| dyro::device | picks a graphics card and creates the directx 12 device |
| dyro::adapter_selection | enumerates and scores all graphics cards |
| dyro::command_queue | submits work to the gpu and synchronizes cpu/gpu |
| dyro::swap_chain | owns the back buffers that end up on screen |
| dyro::descriptor_heap | hands out descriptor slots (texture views) |
| dyro::shader_library | loads compiled shaders (*.cso) from disk |
| dyro::pso_cache | saves/loads pipeline state objects to/from disk |
| dyro::texture_loader | decodes image files / raw pixels and uploads them to the gpu |
| dyro::font | describes a fixed-grid bitmap font atlas for draw_text |
| dyro::renderer_2d | draws textured quads (this is a 2D engine: everything is a quad) |

Every class name above links to its documentation, and from there the
**source browser** shows you the full header — the headers are written to be
read.

## Graphics card selection

Like Unreal, the engine looks at every graphics card in the machine before
picking one (dyro::adapter_selection). Each adapter gets a score based on
its dedicated video memory (the dominant factor), maximum feature level and
highest shader model. Software adapters (WARP) score zero and are only used
when no real card exists. Set `engine_settings::gpu_preference` to
dyro::adapter_preference::lowest_score to run on the weakest card — handy
for testing how your game behaves on low-end hardware. The scoring of every
card is printed to the console at startup.

## Shader compilation (the toolchain)

DirectX 12 can only load *compiled* shaders. The `shader_compiler` tool
(built from `source/tools/shader_compiler`, using Microsoft's DXC compiler)
turns `.hlsl` files into `.cso` bytecode that dyro::shader_library loads at
startup.

It is wired into the build by `DYRO_COMPILE_SHADERS` in the root
`CMakeLists.txt`: every `*.hlsl` file in `/shaders` is compiled into
`<build>/<config>/content/shaders/` as part of a normal build. CMake tracks
the dependencies, so a shader is only recompiled when its source changed —
exactly like C++ files. The shader profile is derived from the file name:
`foo_vs.hlsl` becomes a vertex shader, `foo_ps.hlsl` a pixel shader. To add
a shader, drop the file in `/shaders` and build.

## PSO caching

Creating a pipeline state object makes the gpu driver compile shader
bytecode into real gpu instructions, which is slow. dyro::pso_cache wraps
directx's `ID3D12PipelineLibrary`: pipelines created during a run are
serialized to `cache/pso_cache.bin` on shutdown and loaded instantly on the
next start. The cache invalidates itself when the driver, gpu or a shader
changed — the pipeline is then simply rebuilt and the cache refreshed.
Watch the console: `created from scratch` on the first run, `loaded from
the pso cache` after.

## Textures

dyro::texture_loader uses stb_image, so png/jpg/bmp/tga all work. The gpu
cannot read cpu memory directly: pixels are written into an *upload
buffer*, the gpu copies them into the final texture resource, and a *shader
resource view* is created so shaders can sample it. All of that lives in
one readable function: dyro::texture_loader::create_from_pixels.

## Text

dyro::renderer_2d::draw_text renders with a bitmap font: one atlas texture
holding every character in a fixed grid (`content/fonts/font_8x8.png`,
based on the public domain [font8x8](https://github.com/dhepper/font8x8) by
Daniel Hepper). The dyro::font struct describes the grid; each character
becomes one quad that shows its part of the atlas — text rendering is just
sprite sheet drawing.

## The frame

dyro::renderer_2d owns the frame loop plumbing. One unit quad lives in gpu
memory; every `draw_sprite` call draws that same quad with its own
transform, texture and tint (passed as root constants + a descriptor
table). The engine keeps one command allocator per back buffer and uses a
fence (dyro::command_queue) to wait until the gpu released a buffer before
recording into it again — the classic "frames in flight" pattern, in its
smallest possible form.
