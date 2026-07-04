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

### Convenience scripts

If you'd rather skip Visual Studio and the raw CMake commands, the repo root
has a few `.bat` wrappers around the same workflow:

| script | what it does |
|---|---|
| `generate.bat` | runs `cmake --preset vs2022` to (re)generate `build/dyro_engine.sln` |
| `build.bat [-debug\|-release]` | builds that solution via `cmake --build`; defaults to debug |
| `run.bat [-debug\|-release] [-game=name]` | launches the built `dyro_game.exe` from `build/<config>`; pass `-game=name` to run a different executable, e.g. `-game=awesome_game` |

And a second one for the docs you're reading right now:

| script | what it does |
|---|---|
| `docs.bat` | opens `docs/html/index.html` in your browser (run `docs/generate_docs.bat` first if it doesn't exist yet) |

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
| `sampler_filter` | how textures are filtered when scaled; dyro::texture_filter::nearest keeps pixel art crisp instead of blurring it |

## Where to put your code

Look at `source/games/dyro_game`: it contains the demo game — replace it with
your own. The four dyro::game overrides are all optional; override only the
ones you need. From here, continue with @ref page_drawing and @ref page_input.

## Adding a second game

Every game lives in its own folder under `source/games`, next to the demo
(`source/games/dyro_game`). `source/games/CMakeLists.txt` lists one
`ADD_SUBDIRECTORY(...)` per game — that is the *only* file you touch outside
your new game's own folder. Here is everything needed to add one, using
`awesome_game` as the example name.

1. **Create the folder and its files**:

   ```
   source/games/awesome_game/
     CMakeLists.txt
     private/
       main.cpp
       awesome_game.h
       awesome_game.cpp
   ```

   `awesome_game.h`/`.cpp` hold your `dyro::game` subclass (see "A minimal
   complete game" above); `main.cpp` follows the same `main()` pattern, with
   its own `window_title`.

2. **Add `source/games/awesome_game/CMakeLists.txt`**, modeled on
   `source/games/dyro_game/CMakeLists.txt` with the target renamed
   throughout — this name is also what you pass to `run.bat -game=`:

   ```cmake
   MESSAGE(STATUS "Adding game - awesome_game")
   ADD_EXECUTABLE(awesome_game)

   TARGET_SOURCES(awesome_game PRIVATE
       ${CMAKE_CURRENT_LIST_DIR}/private/main.cpp
       ${CMAKE_CURRENT_LIST_DIR}/private/awesome_game.h
       ${CMAKE_CURRENT_LIST_DIR}/private/awesome_game.cpp)

   GROUPSOURCES(${CMAKE_CURRENT_LIST_DIR}/private private)

   TARGET_LINK_LIBRARIES(awesome_game PRIVATE dyro_engine)

   # Compile all shaders in the /shaders folder as part of the build
   DYRO_COMPILE_SHADERS(awesome_game)

   # Wait for the shared /content folder to be copied next to the exe
   ADD_DEPENDENCIES(awesome_game copy-content)

   SET_TARGET_PROPERTIES(awesome_game PROPERTIES
                         FOLDER "games"
                         VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:awesome_game>")
   ```

   `DYRO_COMPILE_SHADERS` and `copy-content` are both safe to depend on from
   more than one game: shaders and content live in one shared `/shaders` and
   `/content` folder, so every game shares the same compiled/copied output
   instead of redoing it per game.

3. **Register the folder in `source/games/CMakeLists.txt`** — the one line
   this whole process adds outside your game's own folder:

   ```cmake
   ADD_SUBDIRECTORY(dyro_game)
   ADD_SUBDIRECTORY(awesome_game)
   ```

4. **Regenerate and build**: `generate.bat` (or `cmake --preset vs2022`),
   then `build.bat`. Visual Studio will show both `dyro_game` and
   `awesome_game` under the "games" folder; the existing startup project is
   still `dyro_game` — right-click `awesome_game` → *Set as Startup Project*
   to debug it instead, or just run it without opening Visual Studio at all:

   ```
   run.bat -game=awesome_game
   ```

Everything else — the engine library, the shared `/content` and `/shaders`
folders, `dyro::game`, `dyro::engine` — is exactly the same for every game
in the project.
