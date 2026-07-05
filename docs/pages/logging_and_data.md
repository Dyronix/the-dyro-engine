# Logging and asserts {#page_logging_and_data}

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

Logging is compiled in for **debug builds only**. In release every log call
collapses to nothing, so no formatting cost is paid.

## Asserts (core/assert.h)

Use DYRO_ASSERT to document assumptions your code makes; when the condition
is false the program halts in the debugger at the call site:

```cpp
DYRO_ASSERT(frame_index < frame_count);
DYRO_ASSERT_MSG(frame_index < frame_count, "frame {} out of range", frame_index);
```

Like logging, asserts are compiled away in release builds. **The condition
is not even evaluated**, so never put code with side effects inside one.

For errors the game cannot recover from (in any build), dyro::fatal_error
logs the message, shows a message box and exits:

```cpp
if (!m_ball_texture)
{
    dyro::fatal_error("could not load {}", path.string());
}
```
