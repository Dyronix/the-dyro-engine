# Textures and fonts {#page_textures_and_fonts}

## Loading an image file

buz::texture_loader decodes image files (png, jpg, bmp, tga, ...) and
uploads them to the gpu. Load your textures once, in
buz::game::initialize:

```cpp
void my_game::initialize(buz::engine& engine)
{
    m_ball_texture = engine.load_texture("textures/ball.png");
}
```

buz::engine::load_texture takes a path relative to the content folder, which
is where almost every texture lives, so you don't have to juggle paths on your
first line of code. When you do need a file outside the content folder, reach for
buz::texture_loader::load_from_file through `engine.get_texture_loader()` and
pass it a full path (buz::paths::get_content_directory is what
`load_texture` prepends for you).

Textures come back as `std::shared_ptr<buz::texture>`. Store them as
members and pass them to `draw_sprite` every frame. If the file cannot be
read (a wrong path is the usual cause), an error is logged to the console
and a magenta-and-black checkerboard placeholder is returned instead, so a
typo in a filename shows up as an obvious magenta sprite on screen rather
than crashing the next `draw_sprite` call.

Put your images in `content/textures`; the build copies the whole `content`
folder next to the executable, and buz::paths::get_content_directory finds it
from there no matter where the game is started from.

## Procedural textures

buz::texture_loader::create_from_pixels uploads raw rgba pixels (4 bytes
per pixel), so you can build textures in code. The demo game generates a
grayscale texture from smooth noise this way:

```cpp
std::shared_ptr<buz::texture> make_noise_texture(buz::texture_loader& loader, uint32_t size)
{
    std::vector<uint8_t> pixels(static_cast<size_t>(size) * size * 4);

    for (uint32_t y = 0; y < size; ++y)
    {
        for (uint32_t x = 0; x < size; ++x)
        {
            // noise_2d returns roughly [-1, 1]; remap to a 0..255 gray
            const float sample = buz::noise_2d(static_cast<float>(x) * 0.05f, static_cast<float>(y) * 0.05f);
            const auto gray = static_cast<uint8_t>((sample * 0.5f + 0.5f) * 255.0f);

            uint8_t* pixel = &pixels[(static_cast<size_t>(y) * size + x) * 4];
            pixel[0] = gray;  // r
            pixel[1] = gray;  // g
            pixel[2] = gray;  // b
            pixel[3] = 255;   // a
        }
    }

    return loader.create_from_pixels(size, size, pixels.data());
}
```

Pixels with alpha 0 are fully transparent. The demo uses that to build a
sprite sheet of circles on a transparent background (see
`source/game/private/demo_game.cpp`).

## Bitmap fonts

buz::renderer_2d::draw_text renders with a buz::font: one atlas texture
holding every character in a fixed grid, in ascii order. The struct only
describes the grid; the defaults match the engine's built-in atlas
(`content/fonts/font_8x8.png`):

```cpp
// in initialize:
m_font.atlas = engine.load_texture("fonts/font_8x8.png");

// in draw:
renderer.draw_text(m_font, "hello", { 20.0f, 20.0f }, 16.0f);
```

Text rendering is just sprite sheet drawing: each character becomes one
quad showing its part of the atlas. To use your own font, draw an atlas
image with the characters in a regular grid and fill in the buz::font
fields (`glyph_size`, `glyphs_per_row`, `first_character`, `glyph_count`)
to match your grid.
