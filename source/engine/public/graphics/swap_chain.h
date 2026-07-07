#pragma once

#include "graphics/descriptor_heap.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <cstdint>

namespace dyx
{
	class device;
	class command_queue;

	//--------------------------------------------------------------
	/// @brief Owns the swap chain and its back buffers.
	///
	/// The swap chain holds the images that end up on screen. While the gpu
	/// renders into one back buffer, the other one is being displayed;
	/// present() swaps their roles.
	class swap_chain
	{
	public:
		// Two buffers: one on screen, one being rendered into
		static constexpr uint32_t k_back_buffer_count = 2;

		//----------------------------------------------------------
		/// @brief Creates the swap chain for a window.
		/// @param graphics_device Device the back buffers are created on.
		/// @param direct_queue Queue that presents frames to the screen.
		/// @param window_handle Window to present into.
		/// @param width Width of the back buffers in pixels.
		/// @param height Height of the back buffers in pixels.
		/// @return True when the swap chain was created successfully.
		bool initialize(device& graphics_device, command_queue& direct_queue, HWND window_handle, uint32_t width, uint32_t height);

		//----------------------------------------------------------
		/// @brief Returns the index of the back buffer we can render into this frame.
		uint32_t get_current_back_buffer_index() const;

		//----------------------------------------------------------
		/// @brief Returns the back buffer resource we can render into this frame.
		ID3D12Resource* get_current_back_buffer() const;

		//----------------------------------------------------------
		/// @brief Returns the render target view of the current back buffer.
		D3D12_CPU_DESCRIPTOR_HANDLE get_current_render_target_view() const;

		//----------------------------------------------------------
		/// @brief Shows the current back buffer on screen (waits for vsync).
		/// @return True when presenting succeeded.
		bool present();

		//----------------------------------------------------------
		/// @brief Returns the width of the back buffers in pixels.
		uint32_t get_width() const { return m_width; }

		//----------------------------------------------------------
		/// @brief Returns the height of the back buffers in pixels.
		uint32_t get_height() const { return m_height; }

		// The format all rendering targets, used when creating pipelines
		static constexpr DXGI_FORMAT k_back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM;

	private:
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swap_chain;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_back_buffers[k_back_buffer_count];

		descriptor_heap m_rtv_heap;

		uint32_t m_width = 0;
		uint32_t m_height = 0;
	};
}
