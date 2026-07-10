# Json files {#page_json}

For settings, save games and level data. buz::json is
[nlohmann::json](https://json.nlohmann.me); buz::load_json and
buz::save_json do the file handling:

```cpp
const auto path = buz::paths::get_executable_directory() / "content" / "settings.json";

buz::json settings = buz::load_json(path);
if (!settings.is_discarded()) // load_json logs an error and returns a discarded value on failure
{
    m_volume = settings["volume"];
    m_player_name = settings["player"]["name"];
}

// saving: build a json value and write it (missing directories are created)
buz::json save;
save["volume"] = m_volume;
save["unlocked_levels"] = { 1, 2, 3 };
buz::save_json(buz::paths::get_executable_directory() / "save" / "save.json", save);
```
