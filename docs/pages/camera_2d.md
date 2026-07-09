# A 2D camera {#page_camera_2d}

> This is a **guided exercise**, not a finished feature. The engine ships with
> no camera on purpose: building one is how you learn the difference between
> *world space* and *screen space*. The pseudo code below is a map, not a
> solution. You write the real C++.

## The problem

Everything you have drawn so far is in **screen pixels**. dyx::renderer_2d::draw_sprite
takes a position, and dyx::make_orthographic_projection (see `core/math.h`) is
locked to the window size with `(0, 0)` fixed at the top-left corner. The
renderer bakes that projection into every draw call:

```cpp
// renderer_2d.cpp, inside submit_quad
constants.transform = m_projection * make_sprite_transform(position, size, rotation_radians);
```

That is fine for a screen full of action. But a side-scroller scrolls: the *level*
is much wider than the *window*, and the view follows the player through it. A
sprite at world x = 5000 has to appear on screen even though the window is only
1280 pixels wide.

The naive fix is to subtract a scroll offset by hand on every draw call:

```cpp
renderer.draw_sprite(*m_enemy, enemy_world_pos - m_scroll, ...);
```

Do that across a whole game and you *will* get it subtly wrong: one layer
scrolls at the wrong speed (broken parallax), the HUD slides off with the
world because you subtracted the offset from it too, a bullet spawns in screen
space but moves in world space. A single, named **camera** removes this whole
category of bug by giving you *one* place that defines "where the world is
relative to the screen."

## The mental model: two coordinate spaces

Keep these two spaces separate in your head, and in your code:

| Space | Origin | What lives here |
|-------|--------|-----------------|
| **World** | Fixed point in the level | The player, enemies, bullets, terrain, pickups |
| **Screen** | Top-left of the window | The HUD: score, health, ammo, minimap |

The camera is nothing more than the **link** between them: a world position
that maps to the top-left of the screen. Converting one to the other is a
subtraction:

```
screen_pos = world_pos - camera.top_left      // world  -> screen (drawing world things)
world_pos  = screen_pos + camera.top_left      // screen -> world (e.g. mouse -> world click)
```

HUD elements skip the conversion entirely. They are *authored* in screen space,
so you draw them at their raw pixel position. That one rule ("world things get
converted, HUD things don't") is the entire discipline.

## Step 1: a `camera2d` value

Start with the smallest thing that works: a struct that just stores where the
camera is looking. Decide up front whether `position` means the **top-left** of
the view or its **center**. Center is friendlier ("the camera looks *at* the
player"), so the examples assume center, and derive the top-left from it.

```
struct camera2d
{
    vec2 position   // world point the camera is centered on
    vec2 viewport   // window size in pixels (width, height)

    // The world point that maps to screen (0, 0)
    function top_left():
        return position - viewport * 0.5

    function world_to_screen(world_pos):
        return world_pos - top_left()

    function screen_to_world(screen_pos):
        return screen_pos + top_left()
}
```

> Think about pixel snapping. If `top_left()` lands on a fractional pixel, a
> pixel-art level shimmers as it scrolls. Once things look right, try flooring
> the offset to whole pixels and see if it looks cleaner.

## Step 2: follow the player

Each frame, in your `update`, move the camera toward the player. The blunt
version snaps the center straight onto the player:

```
camera.position = player.position
```

That is correct but twitchy. Two upgrades, in order of how much you'll want them:

1. **Smoothing.** Ease the camera toward the target instead of snapping. You
   already have dyx::lerp:
   ```
   camera.position = lerp(camera.position, player.position, follow_strength)
   // follow_strength ~ 0.1 per frame = a soft, trailing follow
   ```
2. **A dead zone.** Real side-scrollers only move the camera once the player
   pushes past a box in the middle of the screen, so small movements don't
   slosh the whole world. Only adjust `camera.position` on the axis where the
   player has left the dead-zone rectangle. A dyx::rect built with
   dyx::rect::from_center_size is a natural fit for expressing that box.

## Step 3: clamp to the level

A scrolling level has edges. Without a clamp, the camera walks past the last
screen and you see the void beyond the level. Stop the camera so the view never
leaves the level bounds:

```
// level_bounds is a rect in world space: the full extent of the level
min_center = level_bounds.min + viewport * 0.5
max_center = level_bounds.max - viewport * 0.5
camera.position = clamp(camera.position, min_center, max_center)   // dyx::clamp
```

Watch the case where the level is **smaller** than the viewport on some axis
(a short level, or a tall window): `min_center` can exceed `max_center` and the
clamp flips. Detect it and just center the level on that axis.

## Step 4: draw through the camera

Now the payoff. In `draw`, split your rendering into two passes.

**World pass:** convert every world position through the camera:

```
// terrain, enemies, player, bullets...
renderer.draw_sprite(*enemy_tex, camera.world_to_screen(enemy.position), enemy.size)
renderer.draw_sprite(*player_tex, camera.world_to_screen(player.position), player.size)
```

**HUD pass:** draw last, in raw screen coordinates, *no* conversion:

```
renderer.draw_text(m_font, score_text, {20, 20}, 16)     // stays put while the world scrolls
renderer.draw_rect(health_bar_area, red)                  // screen space, always
```

If the HUD ever drifts with the world, you accidentally ran it through
`world_to_screen`. If the world *doesn't* scroll, you forgot the conversion on
the world pass. Those two symptoms tell you exactly which pass is wrong.

## Parallax, for free

Background layers that scroll *slower* than the foreground sell depth. Because
the camera is one value, parallax is one multiplier: scale the offset per
layer instead of hand-tuning each sprite:

```
function world_to_screen_parallax(world_pos, factor):
    return world_pos - top_left() * factor
    // factor = 1.0 : moves with the world (gameplay layer)
    // factor = 0.5 : distant hills, scroll at half speed
    // factor = 0.0 : locked to the screen (a painted sky)
```

Draw far layers first with a small factor, the gameplay layer at `1.0`, then
the HUD. One camera, one knob per layer, instead of a magic number sprinkled
across every background draw.

## Where should the subtraction live?

You have two honest options. Pick deliberately.

- **In game code (recommended to start).** Keep `camera2d` in your game and
  call `world_to_screen` yourself before each world draw, exactly as above. The
  engine stays untouched, and *you* see every conversion, which is the point
  of the exercise. The cost is discipline: every world draw must remember the
  call.

- **In the renderer (the "real" engine feature).** Give the renderer a view
  offset it applies for you, so `draw_sprite` takes world coordinates directly
  and the HUD uses a "draw in screen space" mode. Look at how `submit_quad`
  composes `m_projection * make_sprite_transform(...)`: a camera is one more
  matrix in that product: a **view** matrix (a translation by `-top_left()`)
  slotted in as `projection * view * model`. You'd add something like
  `set_view(const camera2d&)` and a way to bypass it for HUD draws. This is the
  cleaner API, but touching the renderer is a bigger step. Earn it by making the
  game-code version work first.

Whichever you choose, the concept is identical. The only question is who owns
the subtraction.

## Checklist

- [ ] World things drawn through `world_to_screen`; HUD drawn in raw screen space.
- [ ] Camera follows the player (snap first; add smoothing / dead zone later).
- [ ] Camera clamped to the level bounds, including the level-smaller-than-viewport case.
- [ ] Mouse/click positions converted with `screen_to_world` before hit-testing world objects.
- [ ] (Optional) Parallax factor per background layer.
- [ ] (Optional) Offset snapped to whole pixels for crisp pixel art.

See @ref page_drawing for the draw calls you are wrapping, and @ref page_utilities
for dyx::lerp, dyx::clamp and dyx::rect.
