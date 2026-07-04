#pragma once

#include "core/math.h"
#include "core/rect.h"
#include "graphics/font.h"
#include "graphics/swap_chain.h"
#include "graphics/texture.h"

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>
#include <memory>
#include <string_view>

namespace dyro
{
	class device;
	class command_queue;
	class descriptor_heap;
	class shader_library;
	class pso_cache;
	class texture_loader;

	//--------------------------------------------------------------
	/// @brief How sampled textures are filtered when scaled on screen.
	enum class texture_filter
	{
		linear,  // smooth blending between texels, blurs pixel art when scaled (default)
		nearest, // no blending, keeps hard pixel edges when scaled (pixel art look)
	};

	//--------------------------------------------------------------
	/// @brief Draws textured quads (sprites) into the current back buffer.
	///
	/// This is a 2d engine, so all geometry is a quad: a single unit quad
	/// lives in gpu memory and every draw_sprite call renders it with its
	/// own transformation, texture and tint color.
	///
	/// A frame looks like this:
	/// @code{.cpp}
	/// renderer.begin_frame(clear_color);
	/// renderer.draw_sprite(...);   // as many times as you want
	/// renderer.end_frame();        // executes, presents and synchronizes
	/// @endcode
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
		/// @param textures Loader used to create the renderer's built-in white texture.
		/// @param sampler_filter How textures are filtered when scaled on screen.
		/// @return True when the renderer is ready for use.
		bool initialize(device& graphics_device, command_queue& direct_queue, swap_chain& target_swap_chain, shader_library& shaders, pso_cache& pipeline_cache, descriptor_heap& srv_heap, texture_loader& textures, texture_filter sampler_filter = texture_filter::linear);

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
		void draw_sprite(const texture& sprite_texture, vec2 position, vec2 size, float rotation_radians = 0.0f, const color& tint = color{});

		//----------------------------------------------------------
		/// @brief Draws part of a texture on a quad; this is how sprite
		/// sheets work: put all animation frames in one texture and pick
		/// the source rect of the frame you want to show.
		/// @param sprite_texture Texture to draw part of.
		/// @param source_rect Part of the texture to draw, in texture pixels.
		/// @param position Position of the sprite center in pixels (origin is top-left).
		/// @param size Width and height of the sprite in pixels.
		/// @param rotation_radians Clockwise rotation around the sprite center.
		/// @param tint Color multiplied with the texture (white leaves the texture untouched).
		void draw_sprite(const texture& sprite_texture, const rect& source_rect, vec2 position, vec2 size, float rotation_radians = 0.0f, const color& tint = color{});

		//----------------------------------------------------------
		/// @brief Draws a filled rectangle.
		/// @param area Rectangle to fill, in pixels.
		/// @param fill_color Color of the rectangle.
		void draw_rect(const rect& area, const color& fill_color);

		//----------------------------------------------------------
		/// @brief Draws the outline of a rectangle.
		/// @param area Rectangle to outline, in pixels.
		/// @param thickness Thickness of the outline in pixels, growing inwards.
		/// @param outline_color Color of the outline.
		void draw_rect_outline(const rect& area, float thickness, const color& outline_color);

		//----------------------------------------------------------
		/// @brief Draws a straight line between two points.
		/// @param from Start of the line in pixels.
		/// @param to End of the line in pixels.
		/// @param thickness Thickness of the line in pixels.
		/// @param line_color Color of the line.
		void draw_line(vec2 from, vec2 to, float thickness, const color& line_color);

		//----------------------------------------------------------
		/// @brief Draws a line of text with a bitmap font. Newlines (`'\n'`)
		/// continue on the next line.
		/// @param text_font Font to draw the text with.
		/// @param text The text to draw.
		/// @param position Top-left corner of the first character, in pixels.
		/// @param pixel_height Height of one character in pixels.
		/// @param tint Color of the text.
		void draw_text(const font& text_font, std::string_view text, vec2 position, float pixel_height, const color& tint = color{});

		//----------------------------------------------------------
		/// @brief Finishes the frame: executes the recorded commands and presents.
		void end_frame();

	private:
		//----------------------------------------------------------
		/// @brief All draw calls end up here: draws one quad with the given
		/// texture, uv rect (offset xy + scale zw, in 0..1 texture space),
		/// transformation and tint.
		void submit_quad(const texture& quad_texture, const vec4& uv_rect, vec2 position, vec2 size, float rotation_radians, const color& tint);

		//----------------------------------------------------------
		/// @brief Creates the root signature (describes what data the shaders receive).
		/// @param sampler_filter How the root signature's static sampler filters textures.
		bool create_root_signature(ID3D12Device* d3d_device, texture_filter sampler_filter);

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

		// A single white pixel; drawing it scaled and tinted produces the
		// rectangles and lines of draw_rect/draw_line.
		std::shared_ptr<texture> m_white_texture;

		mat4 m_projection;
	};
}
