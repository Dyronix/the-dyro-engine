#pragma once

#include "graphics/texture.h"

#include <d3d12.h>
#include <wrl/client.h>

#include <filesystem>
#include <memory>

namespace dyro
{
	class device;
	class command_queue;
	class descriptor_heap;

	//--------------------------------------------------------------
	/// @brief Imports image files and uploads them to the gpu as textures.
	///
	/// Uses stb_image to decode the file (png, jpg, bmp, tga, ...) into raw
	/// rgba pixels. The gpu cannot read our cpu memory directly, so the
	/// pixels are first copied into an upload buffer, after which the gpu
	/// copies them into the final texture resource.
	class texture_loader
	{
	public:
		//----------------------------------------------------------
		/// @brief Prepares the loader for uploading textures.
		/// @param graphics_device Device the textures are created on.
		/// @param direct_queue Queue used to execute the upload commands.
		/// @param srv_heap Descriptor heap the shader resource views are allocated from.
		/// @return True when the loader is ready for use.
		bool initialize(device& graphics_device, command_queue& direct_queue, descriptor_heap& srv_heap);

		//----------------------------------------------------------
		/// @brief Loads an image file and uploads it to the gpu.
		/// @param path Path to the image file.
		/// @return The uploaded texture, or nullptr when loading failed.
		std::shared_ptr<texture> load_from_file(const std::filesystem::path& path);

	private:
		device* m_device = nullptr;
		command_queue* m_direct_queue = nullptr;
		descriptor_heap* m_srv_heap = nullptr;

		// Reused for every upload
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_command_allocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command_list;
	};
}
