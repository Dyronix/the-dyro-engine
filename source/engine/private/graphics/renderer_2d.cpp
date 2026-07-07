#include "graphics/renderer_2d.h"

#include "core/log.h"
#include "graphics/command_queue.h"
#include "graphics/d3d_utils.h"
#include "graphics/descriptor_heap.h"
#include "graphics/device.h"
#include "graphics/pso_cache.h"
#include "graphics/shader_library.h"
#include "graphics/texture_loader.h"

#include <cmath>
#include <cstring>

using Microsoft::WRL::ComPtr;

namespace
{
	//--------------------------------------------------------------
	/// @brief A single vertex of the unit quad.
	struct sprite_vertex
	{
		dyx::vec2 position;
		dyx::vec2 uv;
	};

	//--------------------------------------------------------------
	/// @brief Constants sent to the shaders for every sprite (must match sprite_vs.hlsl).
	struct sprite_constants
	{
		dyx::mat4 transform;
		dyx::color tint;
		dyx::vec4 uv_rect; // offset in xy, scale in zw (0..1 texture space)
	};

	constexpr uint32_t sprite_constants_count = sizeof(sprite_constants) / 4; // in 32 bit values
}

namespace dyx
{
	//--------------------------------------------------------------
	bool renderer_2d::initialize(device& graphics_device, command_queue& direct_queue, swap_chain& target_swap_chain, shader_library& shaders, pso_cache& pipeline_cache, descriptor_heap& srv_heap, texture_loader& textures, texture_filter sampler_filter)
	{
		m_device = &graphics_device;
		m_direct_queue = &direct_queue;
		m_swap_chain = &target_swap_chain;
		m_srv_heap = &srv_heap;

		ID3D12Device* d3d_device = graphics_device.get_d3d_device();

		// One command allocator per back buffer, plus one reusable command list
		for (uint32_t index = 0; index < swap_chain::k_back_buffer_count; ++index)
		{
			if (!d3d::verify(d3d_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocators[index])), "CreateCommandAllocator"))
			{
				return false;
			}
		}

		if (!d3d::verify(d3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocators[0].Get(), nullptr, IID_PPV_ARGS(&m_command_list)), "CreateCommandList"))
		{
			return false;
		}

		m_command_list->Close();

		if (!create_root_signature(d3d_device, sampler_filter))
		{
			return false;
		}

		if (!create_pipeline(shaders, pipeline_cache))
		{
			return false;
		}

		if (!create_quad_geometry(d3d_device))
		{
			return false;
		}

		// The built-in white texture behind draw_rect and draw_line
		const uint8_t white_pixel[4] = { 255, 255, 255, 255 };
		m_white_texture = textures.create_from_pixels(1, 1, white_pixel);
		if (m_white_texture == nullptr)
		{
			return false;
		}

		m_projection = make_orthographic_projection(
			static_cast<float>(target_swap_chain.get_width()),
			static_cast<float>(target_swap_chain.get_height()));

		return true;
	}

	//--------------------------------------------------------------
	void renderer_2d::begin_frame(const color& clear_color)
	{
		const uint32_t buffer_index = m_swap_chain->get_current_back_buffer_index();

		// Restart recording into this frame's allocator
		m_command_allocators[buffer_index]->Reset();
		m_command_list->Reset(m_command_allocators[buffer_index].Get(), m_pipeline.Get());

		// The back buffer comes out of present mode and becomes our render target
		const D3D12_RESOURCE_BARRIER to_render_target = d3d::make_transition_barrier(
			m_swap_chain->get_current_back_buffer(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_command_list->ResourceBarrier(1, &to_render_target);

		const D3D12_CPU_DESCRIPTOR_HANDLE render_target_view = m_swap_chain->get_current_render_target_view();
		m_command_list->OMSetRenderTargets(1, &render_target_view, FALSE, nullptr);

		const float clear_values[] = { clear_color.r, clear_color.g, clear_color.b, clear_color.a };
		m_command_list->ClearRenderTargetView(render_target_view, clear_values, 0, nullptr);

		D3D12_VIEWPORT viewport = {};
		viewport.Width = static_cast<float>(m_swap_chain->get_width());
		viewport.Height = static_cast<float>(m_swap_chain->get_height());
		viewport.MaxDepth = 1.0f;
		m_command_list->RSSetViewports(1, &viewport);

		D3D12_RECT scissor = {};
		scissor.right = static_cast<LONG>(m_swap_chain->get_width());
		scissor.bottom = static_cast<LONG>(m_swap_chain->get_height());
		m_command_list->RSSetScissorRects(1, &scissor);

		// All sprites share the same pipeline, quad and texture heap
		m_command_list->SetGraphicsRootSignature(m_root_signature.Get());

		ID3D12DescriptorHeap* descriptor_heaps[] = { m_srv_heap->get_d3d_heap() };
		m_command_list->SetDescriptorHeaps(1, descriptor_heaps);

		m_command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_command_list->IASetVertexBuffers(0, 1, &m_quad_vertex_buffer_view);
		m_command_list->IASetIndexBuffer(&m_quad_index_buffer_view);
	}

	//--------------------------------------------------------------
	void renderer_2d::draw_sprite(const texture& sprite_texture, vec2 position, vec2 size, float rotation_radians, const color& tint)
	{
		// No source rect given, so the uv rect covers the whole texture
		submit_quad(sprite_texture, { 0.0f, 0.0f, 1.0f, 1.0f }, position, size, rotation_radians, tint);
	}

	//--------------------------------------------------------------
	void renderer_2d::draw_sprite(const texture& sprite_texture, const rect& source_rect, vec2 position, vec2 size, float rotation_radians, const color& tint)
	{
		// The source rect is in texture pixels; the shader wants it in
		// 0..1 texture space.
		const vec2 texture_size = {
			static_cast<float>(sprite_texture.get_width()),
			static_cast<float>(sprite_texture.get_height()) };

		const vec2 uv_offset = source_rect.min / texture_size;
		const vec2 uv_scale = source_rect.size() / texture_size;

		submit_quad(sprite_texture, { uv_offset.x, uv_offset.y, uv_scale.x, uv_scale.y }, position, size, rotation_radians, tint);
	}

	//--------------------------------------------------------------
	void renderer_2d::draw_rect(const rect& area, const color& fill_color)
	{
		// A rectangle is just the white texture stretched and tinted
		submit_quad(*m_white_texture, { 0.0f, 0.0f, 1.0f, 1.0f }, area.center(), area.size(), 0.0f, fill_color);
	}

	//--------------------------------------------------------------
	void renderer_2d::draw_rect_outline(const rect& area, float thickness, const color& outline_color)
	{
		const vec2 area_size = area.size();

		// Four thin bars along the edges: top, bottom, left, right. The
		// left and right bars are inset so the corners are not drawn twice
		// (which would show with transparent colors).
		draw_rect({ area.min, { area.max.x, area.min.y + thickness } }, outline_color);
		draw_rect({ { area.min.x, area.max.y - thickness }, area.max }, outline_color);
		draw_rect({ { area.min.x, area.min.y + thickness }, { area.min.x + thickness, area.max.y - thickness } }, outline_color);
		draw_rect({ { area.max.x - thickness, area.min.y + thickness }, area.max - vec2(0.0f, thickness) }, outline_color);
	}

	//--------------------------------------------------------------
	void renderer_2d::draw_line(vec2 from, vec2 to, float thickness, const color& line_color)
	{
		// A line is a long thin quad: as wide as the distance between the
		// two points, rotated to point from one to the other.
		const vec2 difference = to - from;
		const float line_length = length(difference);
		const float angle = std::atan2(difference.y, difference.x);
		const vec2 line_center = (from + to) * 0.5f;

		submit_quad(*m_white_texture, { 0.0f, 0.0f, 1.0f, 1.0f }, line_center, { line_length, thickness }, angle, line_color);
	}

	//--------------------------------------------------------------
	void renderer_2d::draw_text(const font& text_font, std::string_view text, vec2 position, float pixel_height, const color& tint)
	{
		// Scale the glyphs so they are pixel_height tall on screen
		const float scale = pixel_height / static_cast<float>(text_font.glyph_size.y);
		const vec2 glyph_screen_size = vec2(text_font.glyph_size) * scale;

		// draw_sprite positions centers, so the pen starts half a glyph in
		vec2 pen = position + glyph_screen_size * 0.5f;

		for (const char character : text)
		{
			if (character == '\n')
			{
				pen.x = position.x + glyph_screen_size.x * 0.5f;
				pen.y += glyph_screen_size.y;
				continue;
			}

			// Spaces only advance the pen, there is nothing to draw
			if (character != ' ')
			{
				draw_sprite(*text_font.atlas, text_font.get_glyph_source_rect(character), pen, glyph_screen_size, 0.0f, tint);
			}

			pen.x += glyph_screen_size.x;
		}
	}

	//--------------------------------------------------------------
	void renderer_2d::submit_quad(const texture& quad_texture, const vec4& uv_rect, vec2 position, vec2 size, float rotation_radians, const color& tint)
	{
		// Combine the quad transformation with the screen projection
		// (column-vector convention: the projection is applied last, so it
		// comes first in the multiplication).
		sprite_constants constants;
		constants.transform = m_projection * make_sprite_transform(position, size, rotation_radians);
		constants.tint = tint;
		constants.uv_rect = uv_rect;

		m_command_list->SetGraphicsRoot32BitConstants(0, sprite_constants_count, &constants, 0);
		m_command_list->SetGraphicsRootDescriptorTable(1, quad_texture.get_srv_gpu_handle());

		m_command_list->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}

	//--------------------------------------------------------------
	void renderer_2d::end_frame()
	{
		const uint32_t buffer_index = m_swap_chain->get_current_back_buffer_index();

		// The back buffer is done being a render target and can be presented
		const D3D12_RESOURCE_BARRIER to_present = d3d::make_transition_barrier(
			m_swap_chain->get_current_back_buffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT);
		m_command_list->ResourceBarrier(1, &to_present);

		m_command_list->Close();
		m_direct_queue->execute_command_list(m_command_list.Get());

		m_swap_chain->present();

		// Remember how far the gpu has to be before this frame's allocator
		// can be reused, then wait for the frame that previously used the
		// next back buffer.
		m_frame_fence_values[buffer_index] = m_direct_queue->signal();

		const uint32_t next_buffer_index = m_swap_chain->get_current_back_buffer_index();
		m_direct_queue->wait_for_fence_value(m_frame_fence_values[next_buffer_index]);
	}

	//--------------------------------------------------------------
	bool renderer_2d::create_root_signature(ID3D12Device* d3d_device, texture_filter sampler_filter)
	{
		// Parameter 0: the sprite constants (transform + tint), passed as
		// root constants because they change for every draw call.
		D3D12_ROOT_PARAMETER parameters[2] = {};
		parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		parameters[0].Constants.ShaderRegister = 0; // register(b0)
		parameters[0].Constants.Num32BitValues = sprite_constants_count;
		parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		// Parameter 1: the sprite texture, passed as a descriptor table
		// pointing into the shader visible srv heap.
		D3D12_DESCRIPTOR_RANGE texture_range = {};
		texture_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		texture_range.NumDescriptors = 1;
		texture_range.BaseShaderRegister = 0; // register(t0)

		parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		parameters[1].DescriptorTable.NumDescriptorRanges = 1;
		parameters[1].DescriptorTable.pDescriptorRanges = &texture_range;
		parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// A single sampler baked into the root signature; its filter is fixed
		// for the lifetime of the renderer (rebuilding a root signature at
		// runtime just to flip a filter isn't worth the complexity).
		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = sampler_filter == texture_filter::nearest ? D3D12_FILTER_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0; // register(s0)
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_ROOT_SIGNATURE_DESC root_signature_desc = {};
		root_signature_desc.NumParameters = _countof(parameters);
		root_signature_desc.pParameters = parameters;
		root_signature_desc.NumStaticSamplers = 1;
		root_signature_desc.pStaticSamplers = &sampler;
		root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		ComPtr<ID3DBlob> serialized;
		ComPtr<ID3DBlob> error_messages;
		if (FAILED(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error_messages)))
		{
			log::error("Failed to serialize the root signature: {}",
				error_messages != nullptr ? static_cast<const char*>(error_messages->GetBufferPointer()) : "unknown error");
			return false;
		}

		return d3d::verify(d3d_device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&m_root_signature)), "CreateRootSignature");
	}

	//--------------------------------------------------------------
	bool renderer_2d::create_pipeline(shader_library& shaders, pso_cache& pipeline_cache)
	{
		// The vertex layout must match the sprite_vertex struct and the
		// input of sprite_vs.hlsl.
		const D3D12_INPUT_ELEMENT_DESC input_layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_desc = {};
		pipeline_desc.pRootSignature = m_root_signature.Get();
		pipeline_desc.VS = shaders.get_shader("sprite_vs");
		pipeline_desc.PS = shaders.get_shader("sprite_ps");
		pipeline_desc.InputLayout = { input_layout, _countof(input_layout) };
		pipeline_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipeline_desc.NumRenderTargets = 1;
		pipeline_desc.RTVFormats[0] = swap_chain::k_back_buffer_format;
		pipeline_desc.SampleDesc.Count = 1;
		pipeline_desc.SampleMask = UINT_MAX;

		if (pipeline_desc.VS.pShaderBytecode == nullptr || pipeline_desc.PS.pShaderBytecode == nullptr)
		{
			return false;
		}

		// Standard alpha blending so sprites with transparency work
		D3D12_RENDER_TARGET_BLEND_DESC& blend = pipeline_desc.BlendState.RenderTarget[0];
		blend.BlendEnable = TRUE;
		blend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blend.BlendOp = D3D12_BLEND_OP_ADD;
		blend.SrcBlendAlpha = D3D12_BLEND_ONE;
		blend.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		// No culling: sprites mirrored with a negative size stay visible
		pipeline_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		pipeline_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

		// 2d rendering draws back to front, no depth buffer needed
		pipeline_desc.DepthStencilState.DepthEnable = FALSE;
		pipeline_desc.DepthStencilState.StencilEnable = FALSE;

		m_pipeline = pipeline_cache.get_or_create_graphics_pipeline(L"sprite_pipeline", pipeline_desc);
		return m_pipeline != nullptr;
	}

	//--------------------------------------------------------------
	bool renderer_2d::create_quad_geometry(ID3D12Device* d3d_device)
	{
		// A unit quad centered around the origin; draw_sprite scales,
		// rotates and moves it into place. Uv (0, 0) is the top-left
		// corner of the texture.
		const sprite_vertex vertices[] =
		{
			{ { -0.5f, -0.5f }, { 0.0f, 0.0f } }, // top-left
			{ {  0.5f, -0.5f }, { 1.0f, 0.0f } }, // top-right
			{ {  0.5f,  0.5f }, { 1.0f, 1.0f } }, // bottom-right
			{ { -0.5f,  0.5f }, { 0.0f, 1.0f } }, // bottom-left
		};

		const uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };

		// The quad is tiny, so an upload heap (cpu memory the gpu reads
		// directly) keeps this simple. Big meshes would be copied to gpu
		// memory instead, like textures are.
		const D3D12_HEAP_PROPERTIES upload_heap = d3d::make_heap_properties(D3D12_HEAP_TYPE_UPLOAD);

		const D3D12_RESOURCE_DESC vertex_buffer_desc = d3d::make_buffer_desc(sizeof(vertices));
		if (!d3d::verify(d3d_device->CreateCommittedResource(&upload_heap, D3D12_HEAP_FLAG_NONE, &vertex_buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_quad_vertex_buffer)), "CreateCommittedResource (quad vertex buffer)"))
		{
			return false;
		}

		const D3D12_RESOURCE_DESC index_buffer_desc = d3d::make_buffer_desc(sizeof(indices));
		if (!d3d::verify(d3d_device->CreateCommittedResource(&upload_heap, D3D12_HEAP_FLAG_NONE, &index_buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_quad_index_buffer)), "CreateCommittedResource (quad index buffer)"))
		{
			return false;
		}

		void* mapped_data = nullptr;

		m_quad_vertex_buffer->Map(0, nullptr, &mapped_data);
		std::memcpy(mapped_data, vertices, sizeof(vertices));
		m_quad_vertex_buffer->Unmap(0, nullptr);

		m_quad_index_buffer->Map(0, nullptr, &mapped_data);
		std::memcpy(mapped_data, indices, sizeof(indices));
		m_quad_index_buffer->Unmap(0, nullptr);

		m_quad_vertex_buffer_view.BufferLocation = m_quad_vertex_buffer->GetGPUVirtualAddress();
		m_quad_vertex_buffer_view.SizeInBytes = sizeof(vertices);
		m_quad_vertex_buffer_view.StrideInBytes = sizeof(sprite_vertex);

		m_quad_index_buffer_view.BufferLocation = m_quad_index_buffer->GetGPUVirtualAddress();
		m_quad_index_buffer_view.SizeInBytes = sizeof(indices);
		m_quad_index_buffer_view.Format = DXGI_FORMAT_R16_UINT;

		return true;
	}
}
