#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>
#include <utility>

namespace buz
{
	//--------------------------------------------------------------
	/// @brief A 2d texture living in gpu memory, ready to be drawn on sprites.
	///
	/// Textures are created through the texture_loader, which reads an image
	/// file, uploads the pixels to the gpu and creates the shader resource
	/// view (the descriptor shaders use to sample the texture).
	class texture
	{
	public:
		//----------------------------------------------------------
		/// @brief Constructs a texture from an uploaded gpu resource.
		/// @param resource Gpu resource holding the pixels.
		/// @param width Width of the texture in pixels.
		/// @param height Height of the texture in pixels.
		/// @param srv_gpu_handle Gpu handle of the shader resource view.
		texture(Microsoft::WRL::ComPtr<ID3D12Resource> resource, uint32_t width, uint32_t height, D3D12_GPU_DESCRIPTOR_HANDLE srv_gpu_handle)
			: m_resource(std::move(resource))
			, m_width(width)
			, m_height(height)
			, m_srv_gpu_handle(srv_gpu_handle)
		{
		}

		//----------------------------------------------------------
		/// @brief Returns the width of the texture in pixels.
		uint32_t get_width() const { return m_width; }

		//----------------------------------------------------------
		/// @brief Returns the height of the texture in pixels.
		uint32_t get_height() const { return m_height; }

		//----------------------------------------------------------
		/// @brief Returns the gpu handle of the shader resource view, used while drawing.
		D3D12_GPU_DESCRIPTOR_HANDLE get_srv_gpu_handle() const { return m_srv_gpu_handle; }

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;

		uint32_t m_width = 0;
		uint32_t m_height = 0;

		D3D12_GPU_DESCRIPTOR_HANDLE m_srv_gpu_handle = {};
	};
}
