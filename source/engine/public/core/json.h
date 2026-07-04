#pragma once

#include <nlohmann/json.hpp>

#include <filesystem>

namespace dyro
{
	// A json value: an object, array, string, number, bool or null. This is
	// nlohmann::json; see https://json.nlohmann.me for how to read values.
	using json = nlohmann::json;

	//--------------------------------------------------------------
	/// @brief Loads and parses a json text file.
	/// @param path Path to the json file.
	/// @return The parsed json. When the file cannot be read or contains
	/// invalid json an error is logged and the returned value reports
	/// true for is_discarded().
	json load_json(const std::filesystem::path& path);

	//--------------------------------------------------------------
	/// @brief Saves a json value to a text file, pretty printed so the
	/// file stays readable in a text editor. Missing directories in the
	/// path are created.
	/// @param path Path the file is written to.
	/// @param data Json value to save.
	/// @return True when the file was written successfully.
	bool save_json(const std::filesystem::path& path, const json& data);
}
