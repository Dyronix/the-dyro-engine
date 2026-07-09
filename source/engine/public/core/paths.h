#pragma once

#include <filesystem>

namespace dyx
{
	namespace paths
	{
		//----------------------------------------------------------
		/// @brief Returns the directory the running executable lives in.
		///
		/// All engine content (compiled shaders, textures, the pipeline
		/// cache, ...) is loaded relative to this directory, so the game
		/// works no matter which working directory it is started from.
		std::filesystem::path get_executable_directory();

		//----------------------------------------------------------
		/// @brief Returns the directory game content is loaded from.
		///
		/// This is @c get_executable_directory() / "content", the folder the
		/// build copies textures, fonts and shaders into next to the
		/// executable. Loading textures relative to it is common enough that
		/// dyx::engine::load_texture wraps this up for you.
		std::filesystem::path get_content_directory();
	}
}
