#pragma once

#include "core/math_2d.h"
#include "graphics/swap_chain.h"
#include "graphics/texture.h"

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>

namespace dyro
{
	class device;
	class command_queue;
	class descriptor_heap;
	class shader_library;
	class pso_cache;

	//--------------------------------------------------------------
	/// @brief Draws textured quads (sprites) into the current back buffer.
	///
	/// This is a 2d engine, so all geometry is a quad: a single unit quad
	/// lives in gpu memory and every draw_sprite call renders it with its
	/// own transformation, texture and tint color.
	///
	/// A frame looks like this:
	///     renderer.begin_frame(clear_color);
	///     renderer.draw_sprite(...);   // as many times as you want
	///     renderer.end_frame();        // executes, presents and synchronizes
	class renderer_2d
	{
	public:
		//----------------------------------------------------------
		/// @brief Creates all rendering resources (root signature, pipeline, quad).
		/// @param graphics_device Device the resources are created on.
		/// @param direct_queue Queue the frame command lists are executed on.
		/// @param target_swap_chain Swap chain that is rendered into.
		/// @param shaders Library the sprite shaders are loaded from.
		/// @param pipeline_cache Cache used to create (or reuse) the sprite pipeline.
		/// @param srv_heap Shader visible heap holding the texture descriptors.
		/// @return True when the renderer is ready for use.
		bool initialize(device& graphics_device, command_queue& direct_queue, swap_chain& target_swap_chain, shader_library& shaders, pso_cache& pipeline_cache, descriptor_heap& srv_heap);

		//----------------------------------------------------------
		/// @brief Starts a new frame: clears the back buffer and prepares for drawing.
		/// @param clear_color Color the screen is cleared with.
		void begin_frame(const color& clear_color);

		//----------------------------------------------------------
		/// @brief Draws a textured quad.
		/// @param sprite_texture Texture to draw on the quad.
		/// @param position Position of the sprite center in pixels (origin is top-left).
		/// @param size Width and height of the sprite in pixels.
		/// @param rotation_radians Clockwise rotation around the sprite center.
		/// @param tint Color multiplied with the texture (white leaves the texture untouched).
		void draw_sprite(const texture& sprite_texture, float2 position, float2 size, float rotation_radians = 0.0f, const color& tint = color{});

		//----------------------------------------------------------
		/// @brief Finishes the frame: executes the recorded commands and presents.
		void end_frame();

	private:
		//----------------------------------------------------------
		/// @brief Creates the root signature (describes what data the shaders receive).
		bool create_root_signature(ID3D12Device* d3d_device);

		//----------------------------------------------------------
		/// @brief Creates the sprite pipeline through the pso cache.
		bool create_pipeline(shader_library& shaders, pso_cache& pipeline_cache);

		//----------------------------------------------------------
		/// @brief Creates the unit quad vertex and index buffers.
		bool create_quad_geometry(ID3D12Device* d3d_device);

		device* m_device = nullptr;
		command_queue* m_direct_queue = nullptr;
		swap_chain* m_swap_chain = nullptr;
		descriptor_heap* m_srv_heap = nullptr;

		// One command allocator per back buffer: an allocator can only be
		// reset when the gpu finished the frame that used it.
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_command_allocators[swap_chain::k_back_buffer_count];
		uint64_t m_frame_fence_values[swap_chain::k_back_buffer_count] = {};

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command_list;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_root_signature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipeline;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_quad_vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_quad_index_buffer;
		D3D12_VERTEX_BUFFER_VIEW m_quad_vertex_buffer_view = {};
		D3D12_INDEX_BUFFER_VIEW m_quad_index_buffer_view = {};

		float4x4 m_projection;
	};
}
