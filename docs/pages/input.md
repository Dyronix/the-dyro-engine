# Input {#page_input}

Keyboard and mouse state lives in buz::input; grab it from the engine.
Both `update` and `draw` receive the engine, so input is always one call
away, no need to stash a pointer:

```cpp
buz::input& input = engine.get_input();
```

## Held, pressed or released?

The three keyboard questions differ in *when* they are true:

```cpp
if (input.is_key_down(buz::key::a))           { /* every frame while held  */ }
if (input.was_key_pressed(buz::key::space))   { /* only the frame it went down */ }
if (input.was_key_released(buz::key::escape)) { /* only the frame it came up  */ }
```

Use `is_key_down` for continuous actions (moving, shooting a beam) and
`was_key_pressed` for one-shot actions (jump, toggle pause). With
`is_key_down` a single tap would fire on every frame it spans.

The available keys are the buz::key enum: letters (`buz::key::a` ...),
digits (`buz::key::num_0` ...), arrows, `space`, `enter`, `escape`,
`shift`, `control`, `tab` and `backspace`.

## Movement

The classic wasd movement, from the demo game. Normalizing the direction
keeps diagonal movement from being faster than straight movement, and
multiplying by `delta_seconds` makes the speed independent of the frame
rate:

```cpp
void demo_game::update(buz::engine& engine, float delta_seconds)
{
    buz::input& input = engine.get_input();

    buz::vec2 direction = { 0.0f, 0.0f };
    if (input.is_key_down(buz::key::a) || input.is_key_down(buz::key::left))  { direction.x -= 1.0f; }
    if (input.is_key_down(buz::key::d) || input.is_key_down(buz::key::right)) { direction.x += 1.0f; }
    if (input.is_key_down(buz::key::w) || input.is_key_down(buz::key::up))    { direction.y -= 1.0f; }
    if (input.is_key_down(buz::key::s) || input.is_key_down(buz::key::down))  { direction.y += 1.0f; }

    if (buz::length(direction) > 0.0f)
    {
        direction = buz::normalize(direction);
    }

    constexpr float speed = 400.0f; // pixels per second
    m_position += direction * speed * delta_seconds;
}
```

## Mouse

```cpp
const buz::vec2 mouse = input.get_mouse_position(); // pixels, top-left origin

if (input.was_mouse_button_pressed(buz::mouse_button::left))
{
    // did the click land on something?
    const buz::rect bounds = buz::rect::from_center_size(m_ball_position, { m_ball_size, m_ball_size });
    if (bounds.contains(mouse)) { /* hit! */ }
}

// the wheel: positive away from you, negative towards you, one "click" is 1
m_zoom += input.get_mouse_wheel_delta() * 0.1f;
```

The same held/pressed/released trio exists for mouse buttons:
buz::input::is_mouse_button_down, buz::input::was_mouse_button_pressed
and buz::input::was_mouse_button_released, with
buz::mouse_button::left / `right` / `middle`.
