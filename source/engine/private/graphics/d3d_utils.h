#pragma once

#include "core/log.h"

#include <d3d12.h>

namespace dyro
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
				log::error("%s failed (hresult 0x%08X)", what, static_cast<unsigned int>(result));
				return false;
			}

			return true;
		}

		//----------------------------------------------------------
		/// @brief Builds a resource barrier that transitions a resource between two states.
		/// @param resource Resource to transition.
		/// @param state_before State the resource is currently in.
		/// @param state_after State the resource should transition to.
		/// @return Filled in transition barrier.
		inline D3D12_RESOURCE_BARRIER make_transition_barrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after)
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = resource;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = state_before;
			barrier.Transition.StateAfter = state_after;

			return barrier;
		}

		//----------------------------------------------------------
		/// @brief Fills in a buffer resource description of the given size.
		/// @param size Size of the buffer in bytes.
		/// @return Filled in buffer resource description.
		inline D3D12_RESOURCE_DESC make_buffer_desc(UINT64 size)
		{
			D3D12_RESOURCE_DESC desc = {};
			desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			desc.Width = size;
			desc.Height = 1;
			desc.DepthOrArraySize = 1;
			desc.MipLevels = 1;
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.SampleDesc.Count = 1;
			desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

			return desc;
		}

		//----------------------------------------------------------
		/// @brief Fills in heap properties for the given heap type.
		/// @param type Heap type (default for gpu memory, upload for cpu-to-gpu memory).
		/// @return Filled in heap properties.
		inline D3D12_HEAP_PROPERTIES make_heap_properties(D3D12_HEAP_TYPE type)
		{
			D3D12_HEAP_PROPERTIES properties = {};
			properties.Type = type;

			return properties;
		}
	}
}
