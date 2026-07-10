#pragma once

#include "application/input.h"
#include "application/window.h"
#include "graphics/adapter_selection.h"
#include "graphics/command_queue.h"
#include "graphics/descriptor_heap.h"
#include "graphics/device.h"
#include "graphics/pso_cache.h"
#include "graphics/renderer_2d.h"
#include "graphics/shader_library.h"
#include "graphics/swap_chain.h"
#include "graphics/texture_loader.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

namespace buz
{
	class game;

	//--------------------------------------------------------------
	/// @brief Settings the engine starts with.
	struct engine_settings
	{
		uint32_t window_width = 1280;
		uint32_t window_height = 720;
		std::wstring window_title = L"BuzEngine";

		// Which graphics card to run on; lowest_score is handy to test how
		// the game behaves on weaker hardware.
		adapter_preference gpu_preference = adapter_preference::highest_score;

		color clear_color = { 0.1f, 0.1f, 0.15f, 1.0f };

		// How textures are filtered when scaled on screen; nearest keeps
		// pixel art crisp instead of blurring it like linear does.
		texture_filter sampler_filter = texture_filter::linear;
	};

	//--------------------------------------------------------------
	/// @brief Owns all engine systems and runs the main loop.
	///
	/// Usage (see the buz_game project for a full example):
	/// @code{.cpp}
	/// my_game game;
	/// buz::engine engine;
	/// return engine.run(game, settings);
	/// @endcode
	class engine
	{
	public:
		//----------------------------------------------------------
		/// @brief Initializes all systems, runs the game until the window closes.
		/// @param active_game The game to run.
		/// @param settings Startup settings.
		/// @return Exit code for main (0 on success).
		int run(game& active_game, const engine_settings& settings);

		//----------------------------------------------------------
		/// @brief Loads a texture from the content folder.
		///
		/// The path is relative to the content directory, so
		/// @code engine.load_texture("textures/ball.png") @endcode
		/// loads @c content/textures/ball.png next to the executable. This is
		/// the shorthand for the common case; call get_texture_loader()
		/// yourself only for a path outside the content folder.
		///
		/// Like buz::texture_loader::load_from_file, a failed load is logged
		/// and returns the magenta placeholder rather than nullptr, so a wrong
		/// path shows up on screen instead of crashing the next draw.
		/// @param content_relative_path Image file path, relative to the content directory.
		/// @return The uploaded texture, or the placeholder when loading failed.
		std::shared_ptr<texture> load_texture(const std::filesystem::path& content_relative_path);

		//----------------------------------------------------------
		/// @brief Returns the texture loader, used to import textures in game::initialize.
		texture_loader& get_texture_loader() { return m_texture_loader; }

		//----------------------------------------------------------
		/// @brief Returns the keyboard and mouse state.
		input& get_input() { return m_input; }

		//----------------------------------------------------------
		/// @brief Returns the window the engine renders into.
		window& get_window() { return m_window; }

		//----------------------------------------------------------
		/// @brief Returns the graphics device.
		device& get_device() { return m_device; }

	private:
		//----------------------------------------------------------
		/// @brief Brings all engine systems up in the right order.
		/// @param settings Startup settings.
		/// @return True when every system initialized successfully.
		bool initialize(const engine_settings& settings);

		window m_window;
		input m_input;
		device m_device;
		command_queue m_direct_queue;
		swap_chain m_swap_chain;
		descriptor_heap m_srv_heap;
		shader_library m_shader_library;
		pso_cache m_pso_cache;
		texture_loader m_texture_loader;
		renderer_2d m_renderer;
	};
}
