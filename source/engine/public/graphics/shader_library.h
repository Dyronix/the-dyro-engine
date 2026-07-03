#pragma once

#include <d3d12.h>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace dyro
{
	//--------------------------------------------------------------
	/// @brief Loads compiled shaders (*.cso files) from disk and keeps them in memory.
	///
	/// Directx 12 can only work with shaders that are already compiled to
	/// bytecode. Our shader_compiler tool compiles every *.hlsl file in the
	/// /shaders folder during the build and stores the result next to the
	/// executable in content/shaders. This class loads those files.
	class shader_library
	{
	public:
		//----------------------------------------------------------
		/// @brief Sets the directory the compiled shaders are loaded from.
		/// @param shader_directory Directory containing the *.cso files.
		/// @return True when the directory exists.
		bool initialize(const std::filesystem::path& shader_directory);

		//----------------------------------------------------------
		/// @brief Returns the bytecode of a compiled shader, loading it on first use.
		/// @param name Shader name without extension (e.g. "sprite_vs").
		/// @return Shader bytecode, or an empty bytecode when the shader could not be loaded.
		D3D12_SHADER_BYTECODE get_shader(const std::string& name);

	private:
		std::filesystem::path m_shader_directory;

		// Keeps the shader bytes alive for as long as the library exists;
		// pipeline creation reads directly from these buffers.
		std::unordered_map<std::string, std::vector<char>> m_shaders;
	};
}
