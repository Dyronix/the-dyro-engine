# Logging and asserts {#page_logging_and_data}

## Logging (core/log.h)

Three severities, all using std::format's `{}` style formatting. The format
string is checked at compile time, so a placeholder/argument mismatch is a
compile error. Messages appear in the console window and in the log file:

```cpp
dyx::log::info("loaded {} textures", texture_count);
dyx::log::warn("health {} is above the maximum, clamping", health);
dyx::log::error("could not read {}", path.string());

// format specifiers work too: {:.2f} = two decimals
dyx::log::info("New ball tint: ({:.2f}, {:.2f}, {:.2f})", tint.r, tint.g, tint.b);
```

Logging is compiled in for **debug builds only**. In release every log call
collapses to nothing, so no formatting cost is paid.

## Asserts (core/assert.h)

Use DYX_ASSERT to document assumptions your code makes; when the condition
is false the program halts in the debugger at the call site:

```cpp
DYX_ASSERT(frame_index < frame_count);
DYX_ASSERT_MSG(frame_index < frame_count, "frame {} out of range", frame_index);
```

Like logging, asserts are compiled away in release builds. **The condition
is not even evaluated**, so never put code with side effects inside one.

For errors the game cannot recover from (in any build), dyx::fatal_error
logs the message, shows a message box and exits:

```cpp
if (!m_ball_texture)
{
    dyx::fatal_error("could not load {}", path.string());
}
```
