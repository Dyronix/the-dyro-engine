# Utilities {#page_utilities}

## Math (core/math.h)

The engine uses [glm](https://github.com/g-truc/glm) under dyro names:
dyro::vec2, dyro::vec3, dyro::vec4, dyro::mat4, ... These are aliases, not
wrappers — every glm function works on them directly, and the most used
ones are available as `dyro::` too:

```cpp
dyro::vec2 to_target = target - position;
float dist = dyro::length(to_target);
dyro::vec2 dir = dyro::normalize(to_target);

position = dyro::clamp(position, dyro::vec2(0.0f), dyro::vec2(1280.0f, 720.0f));
float angle = dyro::radians(45.0f); // degrees -> radians (rotations are in radians)

// lerp: smooth a value towards a target (t = 0 gives a, t = 1 gives b)
m_smoothed_fps = dyro::lerp(m_smoothed_fps, 1.0f / delta_seconds, 0.05f);
```

Colors are dyro::color — four floats `r, g, b, a` in [0, 1], defaulting to
white.

## Rectangles (core/rect.h)

dyro::rect is the workhorse for bounds and collision checks. It stores two
corners (`min` top-left, `max` bottom-right); build one around a sprite
with dyro::rect::from_center_size:

```cpp
const dyro::rect ball_bounds = dyro::rect::from_center_size(m_ball_position, { m_ball_size, m_ball_size });

if (ball_bounds.contains(mouse_position))   { /* point inside rect  */ }
if (ball_bounds.intersects(paddle_bounds))  { /* rects overlap      */ }
```

## Timers (core/timer.h)

A freshly constructed dyro::timer starts measuring immediately:

```cpp
dyro::timer stopwatch;
expensive_work();
dyro::log::info("took {} seconds", stopwatch.elapsed_seconds());
```

## Random numbers (core/random.h)

Fast pseudo random numbers. The generator is deterministic: after
dyro::set_random_seed with the same value it always produces the same
sequence, which makes bugs reproducible.

```cpp
dyro::set_random_seed(1234); // once, in initialize

float chance = dyro::random_float();              // [0, 1)
float x = dyro::random_range(0.0f, 1280.0f);      // [min, max)
int damage = dyro::random_range(3, 5);            // [min, max] both inclusive
```

## Noise (core/noise.h)

Smooth noise for procedural content: terrain heights, cloud patterns,
camera shake, wobbly movement. Unlike `random_float` it has no state — the
same coordinates always return the same value, and nearby coordinates
return similar values:

```cpp
// the noise repeats its features about once per unit, so scale the input
// coordinates to control how fast it changes
float height = dyro::noise_2d(x * 0.01f, y * 0.01f); // slow, smooth

// a common trick: use time as the third coordinate to animate 2d noise
float wobble = dyro::noise_3d(x * 0.05f, y * 0.05f, m_time);
```

Both return values roughly in [-1, 1] — remap with `* 0.5f + 0.5f` when you
need [0, 1] (see the procedural texture example in
@ref page_textures_and_fonts).
