# DyxEngine

A small DirectX 12 engine for 2D games, written to be read. Every system is
one small class with one job, so you can dig into any part of the engine and
understand it completely.

## Documentation

Read the docs online at **<https://dyronix.github.io/the-dyro-engine/>**: the
full searchable API reference plus guide pages with examples for every system,
including how to make your own game, the folder structure and how the engine
works internally.

Maintainers: the site is built by the `Docs` GitHub Actions workflow
(`.github/workflows/docs.yml`) and published to GitHub Pages automatically when
a `v*` version tag is pushed (or on demand from the Actions tab). The generated
html is no longer committed to the repo. To preview your changes before tagging,
run `docs/generate_docs.bat`, which needs [Doxygen](https://www.doxygen.nl/download.html)
on your PATH. (One-time repo setup: Settings → Pages → Build and deployment →
Source = "GitHub Actions".)

## Requirements

- **Visual Studio 2026**, or **Visual Studio 2022** version 17.5 or newer, with
  the *Desktop development with C++* workload. That workload also installs a
  recent **Windows 10/11 SDK**, which the engine needs.
- **CMake** on your PATH. Visual Studio 2026 generation requires **CMake 4.2 or
  newer**; the 2022 toolset works with older CMake versions.
- A **C++20** compiler — the standard is the same one current game consoles
  compile with. The engine uses `std::format`, concepts, ranges and `std::span`
  throughout.

## Supported platforms

Windows 10 and Windows 11 (64-bit) only. The renderer is built directly on
DirectX 12, so there is no macOS or Linux build.

## Building and running

```
generate.bat                 # generates build/dyx_engine.sln (VS2026 by default)
build.bat -debug             # or open the solution and press F5
```

`generate.bat` targets Visual Studio 2026 by default; pass `-2022` to target
Visual Studio 2022 instead. Switching between them re-generates from scratch.

`build.bat` takes `-debug` (default) or `-release`. You can also just open
`build/dyx_engine.sln` in Visual Studio and press F5.

The solution sets `simple_game` as the startup project: a tiny "catch the
ball" game meant as a first project. Run it and slide the paddle with the arrow
keys to catch the falling ball; the score goes up each catch. When you want to
see more of what the engine can do, run the demo instead —
`run.bat -game=dyx_game` — a checkerboard, a rotating red quad, a bouncing ball
and a small hud. Move that ball with wasd, tint it with space, resize it with
the mouse wheel.

`run.bat` launches a built game: `-debug`/`-release` picks the config,
`-game=name` picks the game, and any other `-setting=value` is forwarded to the
game to override an engine setting (for example `run.bat -window_width=1920`).

## Pulling the latest code

```
git pull
generate.bat                 # re-run after pulling so the solution picks up new files
build.bat -debug
```

Re-run `generate.bat` after every pull. New games and new `.cpp`/`.h` files are
picked up automatically, but CMake only regenerates the solution when you run
it. If a pull changed the CMake setup and a build misbehaves, delete the
disposable `build/` folder and run `generate.bat` again for a clean configure.