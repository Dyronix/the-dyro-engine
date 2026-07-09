# Getting started {#page_getting_started}

## Building the project

You need **Visual Studio 2026** (or **2022**, version 17.5 or newer) with the
*Desktop development with C++* workload and a recent **Windows 10/11 SDK**
(both are part of the default workload installation). The engine is written
in **C++20**, the same standard current game consoles compile with.

```
cmake --preset vs2026        # generates build/dyx_engine.sln (vs2022 also works)
cmake --build --preset debug # or open the solution and press F5
```

(The `vs2026` preset requires CMake 4.2 or newer; use the `vs2022` preset on
older CMake versions.)

The solution sets `simple_game` as the startup project: a tiny "catch the
ball" game meant as a first project. Run it and slide the paddle with the
arrow keys to catch the falling ball. When you want to see more of the engine
in one place, run the demo instead: `run.bat -game=dyx_game`. You should
see a checkerboard, a rotating red quad, a bouncing ball and a small hud; move
that ball with wasd, tint it with space, resize it with the mouse wheel.

### Convenience scripts

If you'd rather skip Visual Studio and the raw CMake commands, the repo root
has a few `.bat` wrappers around the same workflow:

| script | what it does |
|---|---|
| `generate.bat [-2022\|-2026]` | (re)generates `build/dyx_engine.sln`; targets Visual Studio 2026 by default, `-2022` selects Visual Studio 2022 (switching between them regenerates from scratch) |
| `build.bat [-debug\|-release]` | builds that solution via `cmake --build`; defaults to debug |
| `run.bat [-debug\|-release] [-game=name]` | launches the built `dyx_game.exe` from `build/<config>`; pass `-game=name` to run a different executable, e.g. `-game=awesome_game` |

These docs are published online at <https://dyronix.github.io/the-dyro-engine/> 
and rebuilt automatically whenever a `v*` version tag is pushed. 
To preview them locally instead, install [Doxygen](https://www.doxygen.nl/download.html) 
and use these two wrappers:

| script | what it does |
|---|---|
| `docs/generate_docs.bat` | builds `docs/html` from the headers and these pages (needs Doxygen on your PATH) |
| `docs.bat` | opens the locally built `docs/html/index.html` in your browser (run `docs/generate_docs.bat` first to generate it) |

Keep in mind that some links and references might be broken in a local build of the documentation.

## A minimal complete game

Everything starts in `main()`: fill in dyx::engine_settings, create your
game and hand both to dyx::engine::run. The engine initializes all systems,
runs the main loop until the window closes, and shuts everything down again.

```cpp
#include "application/engine.h"
#include "application/game.h"

class my_game : public dyx::game
{
public:
    void initialize(dyx::engine& engine) override
    {
        // Paths are relative to the content folder next to the executable, so
        // the game works no matter which working directory it is started from.
        m_ball = engine.load_texture("textures/ball.png");
    }

    void draw(dyx::engine& engine, dyx::renderer_2d& renderer) override
    {
        renderer.draw_sprite(*m_ball, { 640.0f, 360.0f }, { 120.0f, 120.0f });
    }

private:
    std::shared_ptr<dyx::texture> m_ball;
};

int main(int argc, char** argv)
{
    // args::parser lets any setting be overridden at launch (see below). Each
    // value you pass here is the fallback used when the setting is not on the
    // command line, so this doubles as "the default set in code".
    args::parser arguments(argc, argv);

    dyx::engine_settings settings;
    settings.window_width = arguments.get("window_width", 1280u);
    settings.window_height = arguments.get("window_height", 720u);
    settings.window_title = arguments.get("window_title", std::wstring(L"my first game"));

    my_game game;

    dyx::engine engine;
    return engine.run(game, settings);
}
```

A regular `main()` (instead of the win32 `WinMain`) keeps a console window
around. That is where all engine log messages appear.

## engine_settings

| field | what it does |
|---|---|
| `window_width`, `window_height` | size of the drawable area in pixels |
| `window_title` | text in the window title bar (a wide string: `L"..."`) |
| `gpu_preference` | which graphics card to run on; dyx::adapter_preference::lowest_score is handy to test how your game behaves on weaker hardware |
| `clear_color` | the color the screen is cleared with before your game draws |
| `sampler_filter` | how textures are filtered when scaled; dyx::texture_filter::nearest keeps pixel art crisp instead of blurring it |

### Overriding settings at launch

The window size and title can be overridden on the command line by their own
name, without rebuilding. `run.bat` forwards any `-name=value` argument to the
game:

```
run.bat -game=my_game -window_width=1920 -window_height=1080 -window_title=Demo
```

Settings you do not pass keep the value set in `main()`. `gpu_preference`,
`clear_color` and `sampler_filter` are set in code and are not exposed on the
command line (they rarely change between runs). A misspelled or unsupported
setting is logged as a warning and its code default is used. The parsing lives
in the small header-only `args` library (`source/third_party/args`).

## Where to put your code

Two games ship with the repo: `source/games/simple_game` (a tiny "catch the
ball" starter, the smallest complete example) and `source/games/dyx_game` (the
feature-touring demo). Start from whichever fits, and replace it with your own.
The four dyx::game overrides are all optional; override only the ones you need.
From here, continue with @ref page_drawing and @ref page_input.

## Adding files to your game

Game sources are discovered automatically: every `.cpp`/`.h` file under your
game's `private` folder is part of the game. To add a class:

1. Create the file(s) in `source/games/<your_game>/private`, from the file
   explorer, or in Visual Studio via right-click on the project → *Add* →
   *New Item...*.
2. Build. CMake notices the new file, regenerates the project, and Visual
   Studio asks to reload; click *Reload All*, then build again.

No `CMakeLists.txt` editing, no re-running `generate.bat`. Removing or
renaming a file works exactly the same way: do it on disk, then build.

**One trap to avoid**: the *Add* → *New Item...* dialog defaults to the
project directory inside `build/` (e.g. `build/source/games/dyx_game`). The
`build/` folder is generated; it can be deleted and rebuilt at any time, so
a file created there is not really part of your game. Always browse to the
real source folder, `source/games/<your_game>/private`, before clicking
*Add*. If a file does end up in `build/`, every build prints this warning in
the Error List until you move it, so nothing is lost silently:

```
warning : This file was created inside the disposable build/ folder - fine
for a quick test, but it is NOT part of your game and will be lost when the
project regenerates. Move it to source/games/<your_game>/private/ - run
docs.bat and read "Adding files to your game" in the getting started guide.
```

The warning starts with the full path of the stray file; double-click it in
the Error List to open the file, then move it to your game's `private`
folder and build again.

## Adding a another game

Every game lives in its own folder under `source/games`, next to the demo
(`source/games/dyx_game`) and the simple game (`source/games/simple_game`). 
`source/games/CMakeLists.txt` lists one `ADD_SUBDIRECTORY(...)` per game. 
That is the *only* file you touch outside your new game's own folder. 
Here is everything needed to add one, using `awesome_game` as the example name.

1. **Create the folder and its files**:

   ```
   source/games/awesome_game/
     CMakeLists.txt
     private/
       main.cpp
       awesome_game.h
       awesome_game.cpp
   ```

   `awesome_game.h`/`.cpp` hold your `dyx::game` subclass (see "A minimal
   complete game" above); `main.cpp` follows the same `main()` pattern, with
   its own `window_title`. Create these files before the generate step below:
   the build takes whatever is in `private` when it runs, and a game with an
   empty `private` folder fails to configure (no source files).

2. **Add `source/games/awesome_game/CMakeLists.txt`**, modeled on
   `source/games/dyx_game/CMakeLists.txt` with the target renamed
   throughout. This name is also what you pass to `run.bat -game=`:

   ```cmake
   MESSAGE(STATUS "Adding game - awesome_game")
   ADD_EXECUTABLE(awesome_game)

   # Sources: every .cpp/.h file under the private/ folder is part of this game
   FILE(GLOB_RECURSE game_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_LIST_DIR}/private/*.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/private/*.h"
        "${CMAKE_CURRENT_LIST_DIR}/private/*.hpp")
   TARGET_SOURCES(awesome_game PRIVATE ${game_sources})

   GROUPSOURCES(${CMAKE_CURRENT_LIST_DIR}/private private)

   TARGET_LINK_LIBRARIES(awesome_game PRIVATE dyx_engine)

   # Compile all shaders in the /shaders folder as part of the build
   DYX_COMPILE_SHADERS(awesome_game)

   # Wait for the shared /content folder to be copied next to the exe
   ADD_DEPENDENCIES(awesome_game copy-content)

   # Warn on every build about source files accidentally created in build/
   ADD_DEPENDENCIES(awesome_game check_stray_sources)

   SET_TARGET_PROPERTIES(awesome_game PROPERTIES
                         FOLDER "games"
                         VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:awesome_game>")
   ```

   Note the sources block: you will never edit this file again to add code.
   See "Adding files to your game" above. `DYX_COMPILE_SHADERS` and
   `copy-content` are both safe to depend on from more than one game: shaders
   and content live in one shared `/shaders` and `/content` folder, so every
   game shares the same compiled/copied output instead of redoing it per
   game.

3. **Register the folder in `source/games/CMakeLists.txt`**. This is the one
   line this whole process adds outside your game's own folder:

   ```cmake
   ADD_SUBDIRECTORY(simple_game)
   ADD_SUBDIRECTORY(dyx_game)
   ADD_SUBDIRECTORY(awesome_game)
   ```

4. **Regenerate and build**: `generate.bat` (or `cmake --preset vs2026`),
   then `build.bat`. Regenerating is only needed here because a whole new
   game (a new `ADD_SUBDIRECTORY` line) was added; adding files to an
   existing game never needs it. Visual Studio will show both `dyx_game` and
   `awesome_game` under the "games" folder; the existing startup project is
   still `dyx_game`. Right-click `awesome_game` → *Set as Startup Project*
   to debug it instead, or just run it without opening Visual Studio at all:

   ```
   run.bat -game=awesome_game
   ```

Everything else is exactly the same for every game in the project: the
engine library, the shared `/content` and `/shaders` folders, `dyx::game`,
and `dyx::engine`.
