# Logging and asserts {#page_logging_and_data}

## Logging (core/log.h)

Three severities, all using std::format's `{}` style formatting. The format
string is checked at compile time, so a placeholder/argument mismatch is a
compile error. Messages appear in the console window and in the log file:

```cpp
buz::log::info("loaded {} textures", texture_count);
buz::log::warn("health {} is above the maximum, clamping", health);
buz::log::error("could not read {}", path.string());

// format specifiers work too: {:.2f} = two decimals
buz::log::info("New ball tint: ({:.2f}, {:.2f}, {:.2f})", tint.r, tint.g, tint.b);
```

Logging is compiled in for **debug builds only**. In release every log call
collapses to nothing, so no formatting cost is paid.

## Asserts (core/assert.h)

Use BUZ_ASSERT to document assumptions your code makes; when the condition
is false the program halts in the debugger at the call site:

```cpp
BUZ_ASSERT(frame_index < frame_count);
BUZ_ASSERT_MSG(frame_index < frame_count, "frame {} out of range", frame_index);
```

Like logging, asserts are compiled away in release builds. **The condition
is not even evaluated**, so never put code with side effects inside one.

For errors the game cannot recover from (in any build), buz::fatal_error
logs the message, shows a message box and exits:

```cpp
if (!m_ball_texture)
{
    buz::fatal_error("could not load {}", path.string());
}
```
