# Input {#page_input}

Keyboard and mouse state lives in dyro::input; grab it from the engine
(store the engine pointer in `initialize` if you need input in `update`):

```cpp
dyro::input& input = engine.get_input();
```

## Held, pressed or released?

The three keyboard questions differ in *when* they are true:

```cpp
if (input.is_key_down(dyro::key::a))           { /* every frame while held  */ }
if (input.was_key_pressed(dyro::key::space))   { /* only the frame it went down */ }
if (input.was_key_released(dyro::key::escape)) { /* only the frame it came up  */ }
```

Use `is_key_down` for continuous actions (moving, shooting a beam) and
`was_key_pressed` for one-shot actions (jump, toggle pause) — with
`is_key_down` a single tap would fire on every frame it spans.

The available keys are the dyro::key enum: letters (`dyro::key::a` ...),
digits (`dyro::key::num_0` ...), arrows, `space`, `enter`, `escape`,
`shift`, `control`, `tab` and `backspace`.

## Movement

The classic wasd movement, from the demo game. Normalizing the direction
keeps diagonal movement from being faster than straight movement, and
multiplying by `delta_seconds` makes the speed independent of the frame
rate:

```cpp
void demo_game::update(float delta_seconds)
{
    dyro::input& input = m_engine->get_input();

    dyro::vec2 direction = { 0.0f, 0.0f };
    if (input.is_key_down(dyro::key::a) || input.is_key_down(dyro::key::left))  { direction.x -= 1.0f; }
    if (input.is_key_down(dyro::key::d) || input.is_key_down(dyro::key::right)) { direction.x += 1.0f; }
    if (input.is_key_down(dyro::key::w) || input.is_key_down(dyro::key::up))    { direction.y -= 1.0f; }
    if (input.is_key_down(dyro::key::s) || input.is_key_down(dyro::key::down))  { direction.y += 1.0f; }

    if (dyro::length(direction) > 0.0f)
    {
        direction = dyro::normalize(direction);
    }

    constexpr float speed = 400.0f; // pixels per second
    m_position += direction * speed * delta_seconds;
}
```

## Mouse

```cpp
const dyro::vec2 mouse = input.get_mouse_position(); // pixels, top-left origin

if (input.was_mouse_button_pressed(dyro::mouse_button::left))
{
    // did the click land on something?
    const dyro::rect bounds = dyro::rect::from_center_size(m_ball_position, { m_ball_size, m_ball_size });
    if (bounds.contains(mouse)) { /* hit! */ }
}

// the wheel: positive away from you, negative towards you, one "click" is 1
m_zoom += input.get_mouse_wheel_delta() * 0.1f;
```

The same held/pressed/released trio exists for mouse buttons:
dyro::input::is_mouse_button_down, dyro::input::was_mouse_button_pressed
and dyro::input::was_mouse_button_released, with
dyro::mouse_button::left / `right` / `middle`.
