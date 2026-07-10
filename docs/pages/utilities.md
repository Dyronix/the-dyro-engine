# Utilities {#page_utilities}

## Math (core/math.h)

The engine uses [glm](https://github.com/g-truc/glm) under buz names:
buz::vec2, buz::vec3, buz::vec4, buz::mat4, ... These are aliases, not
wrappers. Every glm function works on them directly, and the most used
ones are available as `buz::` too:

```cpp
buz::vec2 to_target = target - position;
float dist = buz::length(to_target);
buz::vec2 dir = buz::normalize(to_target);

position = buz::clamp(position, buz::vec2(0.0f), buz::vec2(1280.0f, 720.0f));
float angle = buz::radians(45.0f); // degrees -> radians (rotations are in radians)

// lerp: smooth a value towards a target (t = 0 gives a, t = 1 gives b)
m_smoothed_fps = buz::lerp(m_smoothed_fps, 1.0f / delta_seconds, 0.05f);
```

Colors are buz::color: four floats `r, g, b, a` in [0, 1], defaulting to
white.

## Rectangles (core/rect.h)

buz::rect is the workhorse for bounds and collision checks. It stores two
corners (`min` top-left, `max` bottom-right); build one around a sprite
with buz::rect::from_center_size:

```cpp
const buz::rect ball_bounds = buz::rect::from_center_size(m_ball_position, { m_ball_size, m_ball_size });

if (ball_bounds.contains(mouse_position))   { /* point inside rect  */ }
if (ball_bounds.intersects(paddle_bounds))  { /* rects overlap      */ }
```

## Timers (core/timer.h)

A freshly constructed buz::timer starts measuring immediately:

```cpp
buz::timer stopwatch;
expensive_work();
buz::log::info("took {} seconds", stopwatch.elapsed_seconds());
```

## Random numbers (core/random.h)

Fast pseudo random numbers. The generator is deterministic: after
buz::set_random_seed with the same value it always produces the same
sequence, which makes bugs reproducible.

```cpp
buz::set_random_seed(1234); // once, in initialize

float chance = buz::random_float();              // [0, 1)
float x = buz::random_range(0.0f, 1280.0f);      // [min, max)
int damage = buz::random_range(3, 5);            // [min, max] both inclusive
```

## Noise (core/noise.h)

Smooth noise for procedural content: terrain heights, cloud patterns,
camera shake, wobbly movement. Unlike `random_float`, it has no state; the
same coordinates always return the same value, and nearby coordinates
return similar values:

```cpp
// the noise repeats its features about once per unit, so scale the input
// coordinates to control how fast it changes
float height = buz::noise_2d(x * 0.01f, y * 0.01f); // slow, smooth

// a common trick: use time as the third coordinate to animate 2d noise
float wobble = buz::noise_3d(x * 0.05f, y * 0.05f, m_time);
```

Both return values roughly in [-1, 1]. Remap with `* 0.5f + 0.5f` when you
need [0, 1] (see the procedural texture example in
@ref page_textures_and_fonts).

## Paths (core/paths.h)

buz::paths::get_executable_directory returns the directory the running
executable lives in. Load **all** content relative to it. A bare relative
path only works when the game happens to be started from the right working
directory, which is not the case when launching from Visual Studio or a
shortcut:

```cpp
const std::filesystem::path content = buz::paths::get_executable_directory() / "content";
m_texture = engine.get_texture_loader().load_from_file(content / "textures" / "ball.png");
```

buz::paths::get_content_directory is the shorthand for that first line
(`get_executable_directory() / "content"`), the folder textures, fonts and
shaders live in. For textures specifically you rarely need either: reach
straight for buz::engine::load_texture, which takes a content-relative path
and prepends the content directory for you.

```cpp
m_texture = engine.load_texture("textures/ball.png");
```
