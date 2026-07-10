#pragma once

#include "core/log.h"

#include <d3d12.h>
#include <wrl/client.h>

namespace buz
{
	namespace d3d
	{
		//----------------------------------------------------------
		/// @brief Checks the result of a d3d call and logs an error when it failed.
		/// @param result Result returned by the d3d call.
		/// @param what Human readable description of the call, used in the error message.
		/// @return True when the call succeeded.
		inline bool verify(HRESULT result, const char* what)
		{
			if (FAILED(result))
			{
				log::error("{} failed (hresult 0x{:08X})", what, static_cast<unsigned int>(result));
				return false;
			}

			return true;
		}

		//----------------------------------------------------------
		/// @brief Temporarily changes whether a d3d debug severity breaks.
		class debug_break_guard
		{
		public:
			debug_break_guard(ID3D12Device* device, D3D12_MESSAGE_SEVERITY severity, BOOL break_enabled, bool enabled = true)
			{
#if defined(_DEBUG)
				if (enabled && SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&m_info_queue))))
				{
					m_severity = severity;
					m_break_on_severity = m_info_queue->GetBreakOnSeverity(severity);
					m_info_queue->SetBreakOnSeverity(severity, break_enabled);
				}
#else
				(void)device;
				(void)severity;
				(void)break_enabled;
				(void)enabled;
#endif
			}

			~debug_break_guard()
			{
#if defined(_DEBUG)
				if (m_info_queue != nullptr)
				{
					m_info_queue->SetBreakOnSeverity(m_severity, m_break_on_severity);
				}
#endif
			}

			debug_break_guard(const debug_break_guard&) = delete;
			debug_break_guard& operator=(const debug_break_guard&) = delete;

		private:
#if defined(_DEBUG)
			Microsoft::WRL::ComPtr<ID3D12InfoQueue> m_info_queue;
			D3D12_MESSAGE_SEVERITY m_severity = D3D12_MESSAGE_SEVERITY_ERROR;
			BOOL m_break_on_severity = FALSE;
#endif
		};

		//----------------------------------------------------------
		/// @brief Builds a resource barrier that transitions a resource between two states.
		/// @param resource Resource to transition.
		/// @param state_before State the resource is currently in.
		/// @param state_after State the resource should transition to.
		/// @return Filled in transition barrier.
		inline D3D12_RESOURCE_BARRIER make_transition_barrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after)
		{
			// Designated initializers (C++20) name each field they set; every
			// field not named stays zero. .Transition picks which member of
			// the barrier's union this initializer fills in.
			return D3D12_RESOURCE_BARRIER
			{
				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
				.Transition =
				{
					.pResource = resource,
					.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
					.StateBefore = state_before,
					.StateAfter = state_after,
				},
			};
		}

		//----------------------------------------------------------
		/// @brief Fills in a buffer resource description of the given size.
		/// @param size Size of the buffer in bytes.
		/// @return Filled in buffer resource description.
		inline D3D12_RESOURCE_DESC make_buffer_desc(UINT64 size)
		{
			return D3D12_RESOURCE_DESC
			{
				.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
				.Width = size,
				.Height = 1,
				.DepthOrArraySize = 1,
				.MipLevels = 1,
				.Format = DXGI_FORMAT_UNKNOWN,
				.SampleDesc = { .Count = 1 },
				.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
			};
		}

		//----------------------------------------------------------
		/// @brief Fills in heap properties for the given heap type.
		/// @param type Heap type (default for gpu memory, upload for cpu-to-gpu memory).
		/// @return Filled in heap properties.
		inline D3D12_HEAP_PROPERTIES make_heap_properties(D3D12_HEAP_TYPE type)
		{
			return D3D12_HEAP_PROPERTIES{ .Type = type };
		}
	}
}
