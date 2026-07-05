# DyroEngine documentation

A small DirectX 12 engine for 2D games, written to be read. Every system is
one small class with one job, so you can dig into any part of the engine and
understand it completely.

Your game is a class that derives from dyro::game and overrides four
functions. The engine owns the main loop and calls them at the right time:

```cpp
class my_game : public dyro::game
{
public:
    void initialize(dyro::engine& engine) override;  // load your textures here
    void update(float delta_seconds) override;       // game logic here
    void draw(dyro::renderer_2d& renderer) override; // draw sprites here
    void shutdown() override;                        // cleanup here
};
```

## Finding things

**Use the search box in the top-right corner.** Type the name of whatever you
are working with, such as `draw_sprite`, `was_key_pressed`, `noise_2d`, or
`engine_settings`, and jump straight to its documentation. Every public
class, function and parameter is in there.

The guide pages below show each system in action, with code you can copy
into your own game:

- @subpage page_getting_started : build the project and write your first game
- @subpage page_drawing : sprites, sprite sheets, rectangles, lines and text
- @subpage page_input : keyboard and mouse
- @subpage page_textures_and_fonts : loading images, procedural textures, bitmap fonts
- @subpage page_utilities : math, rects, timers, random numbers, noise and paths
- @subpage page_logging_and_data : logging and asserts
- @subpage page_json : reading and writing json files
- @subpage page_under_the_hood : how the engine works inside

For the full API reference, browse the class list in the sidebar. Start with
dyro::engine, dyro::renderer_2d and dyro::input, the three classes your game
talks to every frame.
