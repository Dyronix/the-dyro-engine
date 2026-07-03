#include "application/engine.h"

#include "application/game.h"
#include "core/log.h"
#include "core/paths.h"

#include <chrono>

namespace
{
	// Enough descriptor slots for a small 2d game's textures
	constexpr uint32_t srv_heap_capacity = 256;
}

namespace dyro
{
	//--------------------------------------------------------------
	int engine::run(game& active_game, const engine_settings& settings)
	{
		if (!initialize(settings))
		{
			log::error("Engine initialization failed");
			return -1;
		}

		active_game.initialize(*this);

		// The main loop: handle window messages, update the game, draw a
		// frame; repeat until the window is closed.
		auto previous_time = std::chrono::steady_clock::now();

		while (m_window.process_messages())
		{
			const auto current_time = std::chrono::steady_clock::now();
			const float delta_seconds = std::chrono::duration<float>(current_time - previous_time).count();
			previous_time = current_time;

			active_game.update(delta_seconds);

			m_renderer.begin_frame(settings.clear_color);
			active_game.draw(m_renderer);
			m_renderer.end_frame();
		}

		// Make sure the gpu is idle before anything gets destroyed
		m_direct_queue.flush();

		active_game.shutdown();

		// Store the pipelines built this run, so the next run starts faster
		m_pso_cache.save();

		log::info("Engine shut down");
		return 0;
	}

	//--------------------------------------------------------------
	bool engine::initialize(const engine_settings& settings)
	{
		const std::filesystem::path executable_directory = paths::get_executable_directory();

		if (!m_window.initialize(settings.window_width, settings.window_height, settings.window_title))
		{
			return false;
		}

		if (!m_device.initialize(settings.gpu_preference))
		{
			return false;
		}

		if (!m_direct_queue.initialize(m_device.get_d3d_device(), D3D12_COMMAND_LIST_TYPE_DIRECT))
		{
			return false;
		}

		if (!m_swap_chain.initialize(m_device, m_direct_queue, m_window.get_handle(), m_window.get_width(), m_window.get_height()))
		{
			return false;
		}

		// One shader visible heap holds the descriptors of all textures
		if (!m_srv_heap.initialize(m_device.get_d3d_device(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, srv_heap_capacity, true))
		{
			return false;
		}

		if (!m_shader_library.initialize(executable_directory / "content" / "shaders"))
		{
			return false;
		}

		if (!m_pso_cache.initialize(m_device.get_d3d_device(), executable_directory / "cache" / "pso_cache.bin"))
		{
			return false;
		}

		if (!m_texture_loader.initialize(m_device, m_direct_queue, m_srv_heap))
		{
			return false;
		}

		if (!m_renderer.initialize(m_device, m_direct_queue, m_swap_chain, m_shader_library, m_pso_cache, m_srv_heap))
		{
			return false;
		}

		log::info("Engine initialized ({}x{})", settings.window_width, settings.window_height);
		return true;
	}
}
