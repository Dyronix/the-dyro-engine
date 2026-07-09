# Drawing {#page_drawing}

All drawing happens in your dyx::game::draw override, through the
dyx::renderer_2d the engine passes you. The engine already called
dyx::renderer_2d::begin_frame (which clears the screen) before your draw,
and calls dyx::renderer_2d::end_frame (which presents the frame) after it.
Inside `draw` you only submit what you want to see.

## Coordinates

Everything is measured in **pixels**, with `(0, 0)` in the **top-left**
corner of the window and the y axis pointing **down**. Sprite positions are
the **center** of the sprite. Sprites are drawn in the order you submit
them, so draw the background first and the hud last.

## Sprites

```cpp
void my_game::draw(dyx::engine& engine, dyx::renderer_2d& renderer)
{
    // texture, center position, size (all in pixels)
    renderer.draw_sprite(*m_background, { 640.0f, 360.0f }, { 1280.0f, 720.0f });

    // optional: rotation (radians, clockwise) and a tint color
    renderer.draw_sprite(*m_ship, m_ship_position, { 64.0f, 64.0f },
        m_ship_angle, { 1.0f, 0.3f, 0.3f, 1.0f }); // tinted red
}
```

The tint is multiplied with the texture: white (the default) leaves the
texture untouched, and the alpha component makes the sprite transparent.
See dyx::color; components are in the [0, 1] range.

## Sprite sheets

Put all animation frames in one texture and draw only the part you need by
passing a source dyx::rect in **texture pixels**. Picking the frame from
the elapsed time animates the sprite (from the demo game):

```cpp
// Four 64x64 frames laid out next to each other, 8 frames per second
const auto frame = static_cast<uint32_t>(m_time * 8.0f) % 4;
const dyx::rect frame_rect = {
    { static_cast<float>(frame) * 64.0f, 0.0f },
    { static_cast<float>(frame + 1) * 64.0f, 64.0f } };

renderer.draw_sprite(*m_circle_sheet, frame_rect, { 1030.0f, 200.0f }, { 100.0f, 100.0f });
```

That inline timing math is fine for a single strip. Once a character needs
several cycles (idle, run, jump, die), wrap it in a small animator. See
@ref page_sprite_animation for a guided build.

## Rectangles and lines

Useful for debug overlays, health bars and simple shapes. No texture is
needed:

```cpp
renderer.draw_rect(area, { 0.2f, 0.6f, 1.0f, 1.0f });        // filled
renderer.draw_rect_outline(area, 2.0f, { 1.0f, 0.2f, 0.2f, 1.0f }); // 2px outline, growing inwards
renderer.draw_line(from, to, 2.0f, { 1.0f, 1.0f, 0.2f, 0.4f });     // 2px line
```

`area` is a dyx::rect; build one around a sprite with
dyx::rect::from_center_size, which matches how sprites are positioned.

## Text

dyx::renderer_2d::draw_text renders with a bitmap font
(see @ref page_textures_and_fonts for how fonts work):

```cpp
renderer.draw_text(m_font, "hello", { 20.0f, 20.0f }, 16.0f);

// newlines continue on the next line; std::format builds dynamic text
renderer.draw_text(m_font,
    std::format("fps {:.0f}\nscore {}", m_smoothed_fps, m_score),
    { 20.0f, 48.0f }, 16.0f, { 0.6f, 0.8f, 1.0f, 1.0f });
```

The position is the **top-left** corner of the first character (unlike
sprites, which are positioned by their center), and `pixel_height` is the
height of one character.
