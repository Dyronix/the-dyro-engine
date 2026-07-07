#include "graphics/command_queue.h"

#include "graphics/d3d_utils.h"

namespace dyx
{
	//--------------------------------------------------------------
	bool command_queue::initialize(ID3D12Device* d3d_device, D3D12_COMMAND_LIST_TYPE type)
	{
		const D3D12_COMMAND_QUEUE_DESC queue_desc = { .Type = type };

		if (!d3d::verify(d3d_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&m_queue)), "CreateCommandQueue"))
		{
			return false;
		}

		if (!d3d::verify(d3d_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "CreateFence"))
		{
			return false;
		}

		m_fence_event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
		if (m_fence_event == nullptr)
		{
			log::error("Failed to create the fence event handle");
			return false;
		}

		return true;
	}

	//--------------------------------------------------------------
	command_queue::~command_queue()
	{
		if (m_fence_event != nullptr)
		{
			CloseHandle(m_fence_event);
		}
	}

	//--------------------------------------------------------------
	void command_queue::execute_command_list(ID3D12CommandList* command_list)
	{
		ID3D12CommandList* command_lists[] = { command_list };
		m_queue->ExecuteCommandLists(1, command_lists);
	}

	//--------------------------------------------------------------
	uint64_t command_queue::signal()
	{
		const uint64_t fence_value = m_next_fence_value++;
		m_queue->Signal(m_fence.Get(), fence_value);

		return fence_value;
	}

	//--------------------------------------------------------------
	void command_queue::wait_for_fence_value(uint64_t fence_value)
	{
		if (m_fence->GetCompletedValue() >= fence_value)
		{
			return;
		}

		m_fence->SetEventOnCompletion(fence_value, m_fence_event);
		WaitForSingleObject(m_fence_event, INFINITE);
	}

	//--------------------------------------------------------------
	void command_queue::flush()
	{
		wait_for_fence_value(signal());
	}
}
