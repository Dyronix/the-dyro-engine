#include "graphics/device.h"

#include "core/log.h"
#include "graphics/d3d_utils.h"
#include "graphics/shader_model.h"

using Microsoft::WRL::ComPtr;

namespace
{
	//--------------------------------------------------------------
	/// @brief Enables the directx 12 debug layer in debug builds.
	///
	/// The debug layer validates every api call and prints readable error
	/// messages to the visual studio output window. It is invaluable while
	/// learning directx, but it slows rendering down, so release builds
	/// leave it off.
	void enable_debug_layer()
	{
#if defined(_DEBUG)
		ComPtr<ID3D12Debug> debug_interface;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface))))
		{
			debug_interface->EnableDebugLayer();
			dyro::log::info("Directx 12 debug layer enabled");
		}
#endif
	}

	//--------------------------------------------------------------
	/// @brief Makes the debugger break on directx errors in debug builds.
	void enable_debug_breaks(ID3D12Device* d3d_device)
	{
#if defined(_DEBUG)
		ComPtr<ID3D12InfoQueue> info_queue;
		if (SUCCEEDED(d3d_device->QueryInterface(IID_PPV_ARGS(&info_queue))))
		{
			info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		}
#endif
	}
}

namespace dyro
{
	//--------------------------------------------------------------
	bool device::initialize(adapter_preference preference)
	{
		enable_debug_layer();

		UINT factory_flags = 0;
#if defined(_DEBUG)
		factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

		if (!d3d::verify(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&m_factory)), "CreateDXGIFactory2"))
		{
			return false;
		}

		// Score all graphics cards on this machine and pick one
		const std::vector<adapter_info> adapters = enumerate_and_score_adapters(m_factory.Get());

		const adapter_info* selected = select_adapter(adapters, preference);
		if (selected == nullptr)
		{
			log::error("No directx 12 capable graphics card found");
			return false;
		}

		m_adapter_info = *selected;
		log::info("Selected adapter: {} ({} scoring)",
			m_adapter_info.description,
			preference == adapter_preference::highest_score ? "highest" : "lowest");

		// Create the real device on the selected adapter
		if (!d3d::verify(D3D12CreateDevice(m_adapter_info.adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)), "D3D12CreateDevice"))
		{
			return false;
		}

		enable_debug_breaks(m_device.Get());

		log::info("Device created (feature level: {:X}, shader model: {})",
			static_cast<unsigned int>(m_adapter_info.max_feature_level),
			shader_model_to_string(m_adapter_info.highest_shader_model));

		return true;
	}
}
