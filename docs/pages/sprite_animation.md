# Sprite animation {#page_sprite_animation}

> This is a **guided exercise**, not a finished feature. The engine can already
> draw one frame of a sprite sheet (dyx::renderer_2d::draw_sprite with a source
> dyx::rect); it deliberately ships with *no* animation helper. Building one is
> how you learn to separate **what an animation is** (data) from **where it is
> right now** (state). The pseudo code below is a map, not a solution. You write
> the real C++.

## The problem

Look at how the demo animates its sprite sheet (`demo_game.cpp`, in `draw`):

```cpp
// Four 64x64 frames laid out next to each other, 8 frames per second
const auto frame = static_cast<uint32_t>(m_time * 8.0f) % 4;
const dyx::rect frame_rect = {
    { static_cast<float>(frame) * 64.0f, 0.0f },
    { static_cast<float>(frame + 1) * 64.0f, 64.0f } };
renderer.draw_sprite(*m_circle_sheet, frame_rect, { 1030.0f, 200.0f }, { 100.0f, 100.0f });
```

That is perfect for *one* looping four-frame strip. But a side-scrolling action character
has **idle / run / jump / shoot / die** cycles, each with a different frame
count, a different speed, and different rules: run loops forever, but *die*
plays once and holds the last frame. Express all of that with the line above and
you get a swamp of magic numbers (`8.0f`, `% 4`, `64.0f`) copied and re-tuned at
every draw call, and no way to ask "has the death animation finished?"

A small helper fixes the whole category at once. It splits into two pieces:

| Piece | What it is | Changes? |
|-------|-----------|----------|
| **`sprite_animation`** | The *definition* of a cycle: which frames, how fast, does it loop | Fixed data, authored once |
| **`sprite_animator`** | The *playhead*: which animation is playing and how far into it | Ticks every frame |

Keep those two apart and everything else falls out.

## Step 1: turn a frame index into a source rect

Before anything moves, nail the pure geometry: **frame number → dyx::rect in
texture pixels**. A sheet is a grid, so the demo's single row is just the
`columns = 4, rows = 1` case. Given the frame size and how many columns the
sheet has:

```
function frame_rect(frame_index, frame_size, columns):
    col = frame_index mod columns
    row = frame_index div columns          // integer division; 0 for a single-row strip
    top_left = { col * frame_size.x, row * frame_size.y }
    return rect{ top_left, top_left + frame_size }   // dyx::rect is { min, max }
```

Test this alone first: draw frame 0, 1, 2 by hand and confirm the right cell
shows up. Every later step trusts this function, so make it boring and correct
before adding time.

> The demo hardcodes 64. Store `frame_size` once (a dyx::vec2) instead of
> sprinkling the number across the math. That single change already kills most
> of the magic numbers.

## Step 2: a `sprite_animation` value

Now *describe* a cycle without playing it. Pure data, no time, no mutation:

```
struct sprite_animation
{
    int   first_frame     // index of the first cell in the sheet
    int   frame_count     // how many cells this cycle spans
    float fps             // frames per second (the demo's hard-coded 8.0)
    bool  loop            // true: repeat forever; false: play once and hold
}
```

A whole character is then just a handful of these, authored once:

```
idle  = { first_frame: 0,  frame_count: 4,  fps: 6,  loop: true  }
run   = { first_frame: 4,  frame_count: 6,  fps: 12, loop: true  }
jump  = { first_frame: 10, frame_count: 3,  fps: 10, loop: false }
die   = { first_frame: 13, frame_count: 5,  fps: 8,  loop: false }
```

Notice there is still no notion of "now" here. This struct is the same on
frame 1 and frame 10000. That is the point.

## Step 3: a `sprite_animator` that holds the playhead

The moving part. It remembers *which* animation is playing and *how long* it has
been playing, and it exposes the one thing `draw` needs: the current source
rect.

```
struct sprite_animator
{
    sprite_animation current      // the cycle being played
    float elapsed = 0             // seconds since this cycle started

    // Advance the playhead. Call once per frame from update(delta_seconds).
    function update(delta_seconds):
        elapsed += delta_seconds

    // Which cell of `current` are we on right now?
    function current_frame_index():
        step = int(elapsed * current.fps)          // whole frames elapsed
        if current.loop:
            local = step mod current.frame_count
        else:
            local = min(step, current.frame_count - 1)   // clamp: hold the last frame
        return current.first_frame + local

    // The rect to hand to draw_sprite (reuses Step 1).
    function current_frame_rect(frame_size, columns):
        return frame_rect(current_frame_index(), frame_size, columns)
}
```

The whole trick is `elapsed * fps`: seconds become a frame count, then `mod`
(loop) or `min` (one-shot) turns it into a valid cell. That is the demo's
`m_time * 8.0f % 4`, generalized and named.

> **Why store `elapsed` instead of a frame index?** Time is the source of truth;
> the frame is *derived*. Framerate-independent, and you never have to hand-manage
> "did enough time pass to advance?" You recompute it every call.

## Step 4: knowing when a one-shot is done

A looping run never ends, but *die* and *shoot* do, and gameplay needs to know
(revert to idle after a shot, show the game-over screen after death):

```
function finished():
    if current.loop:
        return false
    return int(elapsed * current.fps) >= current.frame_count
```

Now `update` can react: `if (m_animator.finished()) play(idle)`.

## Step 5: switching animations

Changing state (idle → run when the player moves) means pointing the animator at
a new `sprite_animation` **and resetting the playhead**. Otherwise a 4-frame
idle's leftover `elapsed` lands mid-way through a 6-frame run:

```
function play(animation):
    if animation is current:      // already playing it? do nothing
        return                     // (or the cycle restarts every frame and freezes on frame 0)
    current = animation
    elapsed = 0
```

That early-out is the bug everyone hits first: call `play(run)` unconditionally
every frame the key is held and the animation never leaves frame 0. Guarding on
"is this already the current cycle?" is the fix.

Driving it from input then reads like the state machine it is:

```
// in update(delta_seconds)
if player.is_dead:            m_animator.play(die)
else if player.is_jumping:    m_animator.play(jump)
else if player.is_moving:     m_animator.play(run)
else:                         m_animator.play(idle)

m_animator.update(delta_seconds)
```

## Step 6: draw through the animator

The payoff: `draw` no longer does timing math. It asks the animator for a rect
and hands it to the renderer exactly like the demo does:

```
// frame_size and columns describe the sheet layout
rect src = m_animator.current_frame_rect(frame_size, columns)
renderer.draw_sprite(*m_hero_sheet, src, player.screen_position, player.size)
```

Flip the character to face left by drawing with a negative width (or swapping the
rect's min.x / max.x, depending on how your renderer treats a flipped source),
but get the forward-facing cycle solid first.

## A worked side-scroller character

Putting the pieces together, one sheet drives every state, and the *only* code
that changes per state is which `sprite_animation` you `play`:

| State | loop | ends? | drives |
|-------|------|-------|--------|
| idle  | yes  | never | default when no input |
| run   | yes  | never | movement keys held |
| jump  | no   | on landing / `finished()` | jump pressed |
| shoot | no   | revert to idle on `finished()` | fire pressed |
| die   | no   | hold last frame forever | health reaches 0 |

Everything above is ~40 lines of real C++. The value is not the line count. It
is that *timing, looping, and "is it done?" now live in one place* instead of
being re-derived at every draw call.

## Checklist

- [ ] `frame_rect(index, ...)` tested on its own before any time is involved.
- [ ] `sprite_animation` is pure data (no `elapsed`, no mutation).
- [ ] Animator stores **elapsed time**, not a frame index; the frame is derived.
- [ ] Looping uses `mod`; one-shots `min`/clamp to hold the last frame.
- [ ] `play()` early-outs when the animation is already current (or it freezes on frame 0).
- [ ] `finished()` lets gameplay react to one-shots (revert after shoot, game-over after die).
- [ ] `draw` only calls `current_frame_rect()`: no timing math left in the draw pass.

See @ref page_drawing for the dyx::renderer_2d::draw_sprite call you are feeding,
and @ref page_utilities for dyx::rect and the timing helpers.
