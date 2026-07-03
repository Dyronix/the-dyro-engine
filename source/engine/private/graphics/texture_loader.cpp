#include "graphics/texture_loader.h"

#include "core/log.h"
#include "graphics/command_queue.h"
#include "graphics/d3d_utils.h"
#include "graphics/descriptor_heap.h"
#include "graphics/device.h"

#include <stb/stb_image.h>

#include <cstring>

using Microsoft::WRL::ComPtr;

namespace dyro
{
	//--------------------------------------------------------------
	bool texture_loader::initialize(device& graphics_device, command_queue& direct_queue, descriptor_heap& srv_heap)
	{
		m_device = &graphics_device;
		m_direct_queue = &direct_queue;
		m_srv_heap = &srv_heap;

		ID3D12Device* d3d_device = graphics_device.get_d3d_device();

		if (!d3d::verify(d3d_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator)), "CreateCommandAllocator"))
		{
			return false;
		}

		if (!d3d::verify(d3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocator.Get(), nullptr, IID_PPV_ARGS(&m_command_list)), "CreateCommandList"))
		{
			return false;
		}

		// Command lists are created in the recording state, but the upload
		// code expects a closed list it can reset.
		m_command_list->Close();

		return true;
	}

	//--------------------------------------------------------------
	std::shared_ptr<texture> texture_loader::load_from_file(const std::filesystem::path& path)
	{
		// Decode the image file into raw rgba pixels (8 bits per channel)
		int width = 0;
		int height = 0;
		int channels_in_file = 0;

		stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels_in_file, STBI_rgb_alpha);
		if (pixels == nullptr)
		{
			log::error("Failed to load image \"%s\": %s", path.string().c_str(), stbi_failure_reason());
			return nullptr;
		}

		ID3D12Device* d3d_device = m_device->get_d3d_device();

		// Create the texture resource in gpu memory
		D3D12_RESOURCE_DESC texture_desc = {};
		texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texture_desc.Width = static_cast<UINT64>(width);
		texture_desc.Height = static_cast<UINT>(height);
		texture_desc.DepthOrArraySize = 1;
		texture_desc.MipLevels = 1;
		texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texture_desc.SampleDesc.Count = 1;

		const D3D12_HEAP_PROPERTIES default_heap = d3d::make_heap_properties(D3D12_HEAP_TYPE_DEFAULT);

		ComPtr<ID3D12Resource> texture_resource;
		if (!d3d::verify(d3d_device->CreateCommittedResource(&default_heap, D3D12_HEAP_FLAG_NONE, &texture_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture_resource)), "CreateCommittedResource (texture)"))
		{
			stbi_image_free(pixels);
			return nullptr;
		}

		// Ask the device how the pixel rows must be laid out in the upload
		// buffer (rows are padded to a 256 byte alignment).
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
		UINT row_count = 0;
		UINT64 source_row_size = 0;
		UINT64 upload_size = 0;
		d3d_device->GetCopyableFootprints(&texture_desc, 0, 1, 0, &footprint, &row_count, &source_row_size, &upload_size);

		// Create the upload buffer (cpu-writable memory the gpu can copy from)
		const D3D12_HEAP_PROPERTIES upload_heap = d3d::make_heap_properties(D3D12_HEAP_TYPE_UPLOAD);
		const D3D12_RESOURCE_DESC upload_desc = d3d::make_buffer_desc(upload_size);

		ComPtr<ID3D12Resource> upload_buffer;
		if (!d3d::verify(d3d_device->CreateCommittedResource(&upload_heap, D3D12_HEAP_FLAG_NONE, &upload_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload_buffer)), "CreateCommittedResource (upload buffer)"))
		{
			stbi_image_free(pixels);
			return nullptr;
		}

		// Copy the pixels into the upload buffer, row by row
		char* mapped_data = nullptr;
		if (!d3d::verify(upload_buffer->Map(0, nullptr, reinterpret_cast<void**>(&mapped_data)), "ID3D12Resource::Map"))
		{
			stbi_image_free(pixels);
			return nullptr;
		}

		for (UINT row = 0; row < row_count; ++row)
		{
			std::memcpy(
				mapped_data + footprint.Offset + static_cast<size_t>(row) * footprint.Footprint.RowPitch,
				pixels + static_cast<size_t>(row) * source_row_size,
				source_row_size);
		}

		upload_buffer->Unmap(0, nullptr);
		stbi_image_free(pixels);

		// Record the gpu copy from the upload buffer into the texture
		m_command_allocator->Reset();
		m_command_list->Reset(m_command_allocator.Get(), nullptr);

		D3D12_TEXTURE_COPY_LOCATION copy_destination = {};
		copy_destination.pResource = texture_resource.Get();
		copy_destination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_TEXTURE_COPY_LOCATION copy_source = {};
		copy_source.pResource = upload_buffer.Get();
		copy_source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		copy_source.PlacedFootprint = footprint;

		m_command_list->CopyTextureRegion(&copy_destination, 0, 0, 0, &copy_source, nullptr);

		// Make the texture readable by pixel shaders
		const D3D12_RESOURCE_BARRIER barrier = d3d::make_transition_barrier(texture_resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		m_command_list->ResourceBarrier(1, &barrier);

		m_command_list->Close();
		m_direct_queue->execute_command_list(m_command_list.Get());

		// Wait until the copy finished; only then may the upload buffer be
		// released (which happens automatically when it goes out of scope).
		m_direct_queue->flush();

		// Create the shader resource view so shaders can sample the texture
		const uint32_t srv_index = m_srv_heap->allocate();

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Format = texture_desc.Format;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Texture2D.MipLevels = 1;

		d3d_device->CreateShaderResourceView(texture_resource.Get(), &srv_desc, m_srv_heap->get_cpu_handle(srv_index));

		log::info("Loaded texture \"%s\" (%dx%d)", path.string().c_str(), width, height);

		return std::make_shared<texture>(
			std::move(texture_resource),
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height),
			m_srv_heap->get_gpu_handle(srv_index));
	}
}
