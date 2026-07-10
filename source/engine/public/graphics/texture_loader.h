#pragma once

#include "graphics/texture.h"

#include <d3d12.h>
#include <wrl/client.h>

#include <filesystem>
#include <memory>
#include <span>

namespace buz
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
		///
		/// When the file cannot be loaded (a wrong path is the usual cause) an
		/// error is logged and a magenta-and-black checkerboard placeholder is
		/// returned instead of nullptr, so the game keeps running and the
		/// missing texture shows up on screen instead of crashing a draw call.
		/// @param path Path to the image file.
		/// @return The uploaded texture, or the placeholder when loading failed.
		std::shared_ptr<texture> load_from_file(const std::filesystem::path& path);

		//----------------------------------------------------------
		/// @brief Uploads raw pixels to the gpu, e.g. for procedurally
		/// generated textures.
		///
		/// A std::span is a view over contiguous memory that knows its own
		/// size; a std::vector or a plain array converts to it implicitly.
		/// Because the span carries its length, the loader can verify the
		/// pixel data actually matches width * height, which a raw pointer
		/// could never check.
		/// @param width Width of the texture in pixels.
		/// @param height Height of the texture in pixels.
		/// @param rgba_pixels width * height pixels, 4 bytes (r, g, b, a) each.
		/// @return The uploaded texture, or nullptr when uploading failed.
		std::shared_ptr<texture> create_from_pixels(uint32_t width, uint32_t height, std::span<const uint8_t> rgba_pixels);

	private:
		//----------------------------------------------------------
		/// @brief Returns the shared magenta-and-black placeholder texture,
		/// creating it on first use.
		///
		/// The placeholder is built once and reused for every failed load: the
		/// descriptor heap never reuses slots, so making a fresh texture per
		/// failure would slowly exhaust it.
		/// @return The placeholder texture, or nullptr when even that could not be created.
		std::shared_ptr<texture> get_placeholder_texture();

		device* m_device = nullptr;
		command_queue* m_direct_queue = nullptr;
		descriptor_heap* m_srv_heap = nullptr;

		// Reused for every upload
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_command_allocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command_list;

		// Magenta-and-black checkerboard returned when a load fails; created lazily
		std::shared_ptr<texture> m_placeholder_texture;
	};
}
