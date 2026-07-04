# Logging, asserts and json {#page_logging_and_data}

## Logging (core/log.h)

Three severities, all using fmt's `{}` style formatting. Messages appear in
the console window and in the log file:

```cpp
dyro::log::info("loaded {} textures", texture_count);
dyro::log::warn("health {} is above the maximum, clamping", health);
dyro::log::error("could not read {}", path.string());

// format specifiers work too: {:.2f} = two decimals
dyro::log::info("New ball tint: ({:.2f}, {:.2f}, {:.2f})", tint.r, tint.g, tint.b);
```

Logging is compiled in for **debug builds only** — in release every log call
collapses to nothing, so no formatting cost is paid.

## Asserts (core/assert.h)

Use DYRO_ASSERT to document assumptions your code makes; when the condition
is false the program halts in the debugger at the call site:

```cpp
DYRO_ASSERT(frame_index < frame_count);
DYRO_ASSERT_MSG(frame_index < frame_count, "frame {} out of range", frame_index);
```

Like logging, asserts are compiled away in release builds — **the condition
is not even evaluated**, so never put code with side effects inside one.

For errors the game cannot recover from (in any build), dyro::fatal_error
logs the message, shows a message box and exits:

```cpp
if (!m_ball_texture)
{
    dyro::fatal_error("could not load {}", path.string());
}
```

## Json files (core/json.h)

For settings, save games and level data. dyro::json is
[nlohmann::json](https://json.nlohmann.me); dyro::load_json and
dyro::save_json do the file handling:

```cpp
const auto path = dyro::paths::get_executable_directory() / "content" / "settings.json";

dyro::json settings = dyro::load_json(path);
if (!settings.is_discarded()) // load_json logs an error and returns a discarded value on failure
{
    m_volume = settings["volume"];
    m_player_name = settings["player"]["name"];
}

// saving: build a json value and write it (missing directories are created)
dyro::json save;
save["volume"] = m_volume;
save["unlocked_levels"] = { 1, 2, 3 };
dyro::save_json(dyro::paths::get_executable_directory() / "save" / "save.json", save);
```

## Paths (core/paths.h)

dyro::paths::get_executable_directory returns the directory the running
executable lives in. Load **all** content relative to it — a path like
`"content/textures/ball.png"` only works when the game happens to be
started from the right working directory, which is not the case when
launching from Visual Studio or a shortcut:

```cpp
const std::filesystem::path content = dyro::paths::get_executable_directory() / "content";
m_texture = engine.get_texture_loader().load_from_file(content / "textures" / "ball.png");
```
