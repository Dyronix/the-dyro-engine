# Input {#page_input}

Keyboard and mouse state lives in dyx::input; grab it from the engine.
Both `update` and `draw` receive the engine, so input is always one call
away, no need to stash a pointer:

```cpp
dyx::input& input = engine.get_input();
```

## Held, pressed or released?

The three keyboard questions differ in *when* they are true:

```cpp
if (input.is_key_down(dyx::key::a))           { /* every frame while held  */ }
if (input.was_key_pressed(dyx::key::space))   { /* only the frame it went down */ }
if (input.was_key_released(dyx::key::escape)) { /* only the frame it came up  */ }
```

Use `is_key_down` for continuous actions (moving, shooting a beam) and
`was_key_pressed` for one-shot actions (jump, toggle pause). With
`is_key_down` a single tap would fire on every frame it spans.

The available keys are the dyx::key enum: letters (`dyx::key::a` ...),
digits (`dyx::key::num_0` ...), arrows, `space`, `enter`, `escape`,
`shift`, `control`, `tab` and `backspace`.

## Movement

The classic wasd movement, from the demo game. Normalizing the direction
keeps diagonal movement from being faster than straight movement, and
multiplying by `delta_seconds` makes the speed independent of the frame
rate:

```cpp
void demo_game::update(dyx::engine& engine, float delta_seconds)
{
    dyx::input& input = engine.get_input();

    dyx::vec2 direction = { 0.0f, 0.0f };
    if (input.is_key_down(dyx::key::a) || input.is_key_down(dyx::key::left))  { direction.x -= 1.0f; }
    if (input.is_key_down(dyx::key::d) || input.is_key_down(dyx::key::right)) { direction.x += 1.0f; }
    if (input.is_key_down(dyx::key::w) || input.is_key_down(dyx::key::up))    { direction.y -= 1.0f; }
    if (input.is_key_down(dyx::key::s) || input.is_key_down(dyx::key::down))  { direction.y += 1.0f; }

    if (dyx::length(direction) > 0.0f)
    {
        direction = dyx::normalize(direction);
    }

    constexpr float speed = 400.0f; // pixels per second
    m_position += direction * speed * delta_seconds;
}
```

## Mouse

```cpp
const dyx::vec2 mouse = input.get_mouse_position(); // pixels, top-left origin

if (input.was_mouse_button_pressed(dyx::mouse_button::left))
{
    // did the click land on something?
    const dyx::rect bounds = dyx::rect::from_center_size(m_ball_position, { m_ball_size, m_ball_size });
    if (bounds.contains(mouse)) { /* hit! */ }
}

// the wheel: positive away from you, negative towards you, one "click" is 1
m_zoom += input.get_mouse_wheel_delta() * 0.1f;
```

The same held/pressed/released trio exists for mouse buttons:
dyx::input::is_mouse_button_down, dyx::input::was_mouse_button_pressed
and dyx::input::was_mouse_button_released, with
dyx::mouse_button::left / `right` / `middle`.
