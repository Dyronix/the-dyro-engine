#include "graphics/shader_library.h"

#include "core/log.h"

#include <fstream>

namespace dyx
{
	//--------------------------------------------------------------
	bool shader_library::initialize(const std::filesystem::path& shader_directory)
	{
		if (!std::filesystem::exists(shader_directory))
		{
			log::error("Shader directory \"{}\" does not exist", shader_directory.string());
			return false;
		}

		m_shader_directory = shader_directory;
		return true;
	}

	//--------------------------------------------------------------
	D3D12_SHADER_BYTECODE shader_library::get_shader(const std::string& name)
	{
		// Return the cached bytecode when the shader was loaded before
		const auto shader_it = m_shaders.find(name);
		if (shader_it != m_shaders.end())
		{
			return { shader_it->second.data(), shader_it->second.size() };
		}

		const std::filesystem::path shader_path = m_shader_directory / (name + ".cso");

		std::ifstream file(shader_path, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			log::error("Failed to open compiled shader \"{}\" (did the shader compile during the build?)", shader_path.string());
			return {};
		}

		const std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<char> bytecode(static_cast<size_t>(size));
		file.read(bytecode.data(), size);

		log::info("Loaded shader \"{}\" ({} bytes)", name, static_cast<long long>(size));

		const auto [inserted_it, was_inserted] = m_shaders.emplace(name, std::move(bytecode));
		return { inserted_it->second.data(), inserted_it->second.size() };
	}
}
