#pragma once

#include <filesystem>

namespace dyro
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
	}
}
