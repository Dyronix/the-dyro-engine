# Json files {#page_json}

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
