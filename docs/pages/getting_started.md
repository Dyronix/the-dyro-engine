# Getting started {#page_getting_started}

## Building the project

You need **Visual Studio 2022** with the *Desktop development with C++*
workload and a recent **Windows 10/11 SDK** (both are part of the default
workload installation).

```
cmake --preset vs2022        # generates build/dyro_engine.sln
cmake --build --preset debug # or open the solution and press F5
```

The solution sets `dyro_game` as the startup project. Run it and you should
see a checkerboard, a rotating red quad, a bouncing ball and a small hud —
move the ball with wasd, tint it with space, resize it with the mouse wheel.

## A minimal complete game

Everything starts in `main()`: fill in dyro::engine_settings, create your
game and hand both to dyro::engine::run. The engine initializes all systems,
runs the main loop until the window closes, and shuts everything down again.

```cpp
#include "application/engine.h"
#include "application/game.h"
#include "core/paths.h"

class my_game : public dyro::game
{
public:
    void initialize(dyro::engine& engine) override
    {
        // Content is loaded relative to the executable, so the game works
        // no matter which working directory it is started from.
        const auto content = dyro::paths::get_executable_directory() / "content";
        m_ball = engine.get_texture_loader().load_from_file(content / "textures" / "ball.png");
    }

    void draw(dyro::renderer_2d& renderer) override
    {
        renderer.draw_sprite(*m_ball, { 640.0f, 360.0f }, { 120.0f, 120.0f });
    }

private:
    std::shared_ptr<dyro::texture> m_ball;
};

int main()
{
    dyro::engine_settings settings;
    settings.window_width = 1280;
    settings.window_height = 720;
    settings.window_title = L"my first game";

    my_game game;

    dyro::engine engine;
    return engine.run(game, settings);
}
```

A regular `main()` (instead of the win32 `WinMain`) keeps a console window
around — that is where all engine log messages appear.

## engine_settings

| field | what it does |
|---|---|
| `window_width`, `window_height` | size of the drawable area in pixels |
| `window_title` | text in the window title bar (a wide string: `L"..."`) |
| `gpu_preference` | which graphics card to run on; dyro::adapter_preference::lowest_score is handy to test how your game behaves on weaker hardware |
| `clear_color` | the color the screen is cleared with before your game draws |

## Where to put your code

Look at `source/game`: it contains the demo game — replace it with your own.
The four dyro::game overrides are all optional; override only the ones you
need. From here, continue with @ref page_drawing and @ref page_input.
