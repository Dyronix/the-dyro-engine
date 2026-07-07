#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>

namespace dyx
{
	//--------------------------------------------------------------
	/// @brief Wraps a directx 12 command queue together with its fence.
	///
	/// The gpu executes command lists asynchronously: submitting work only
	/// queues it. The fence is a counter the gpu increments when it reaches
	/// a certain point in the queue, which lets the cpu wait until submitted
	/// work has actually finished.
	class command_queue
	{
	public:
		//----------------------------------------------------------
		/// @brief Creates the command queue and its fence.
		/// @param d3d_device Device used to create the queue.
		/// @param type Type of commands the queue executes (direct, copy, compute).
		/// @return True when the queue was created successfully.
		bool initialize(ID3D12Device* d3d_device, D3D12_COMMAND_LIST_TYPE type);

		//----------------------------------------------------------
		/// @brief Destroys the fence event handle.
		~command_queue();

		//----------------------------------------------------------
		/// @brief Returns the underlying directx 12 command queue.
		ID3D12CommandQueue* get_d3d_command_queue() const { return m_queue.Get(); }

		//----------------------------------------------------------
		/// @brief Submits a command list for execution on the gpu.
		/// @param command_list Closed command list to execute.
		void execute_command_list(ID3D12CommandList* command_list);

		//----------------------------------------------------------
		/// @brief Inserts a fence signal in the queue and returns its value.
		/// @return Fence value that is reached when all previously submitted work finished.
		uint64_t signal();

		//----------------------------------------------------------
		/// @brief Blocks the cpu until the gpu reached the given fence value.
		/// @param fence_value Fence value to wait for, as returned by signal().
		void wait_for_fence_value(uint64_t fence_value);

		//----------------------------------------------------------
		/// @brief Blocks the cpu until the gpu finished all submitted work.
		void flush();

	private:
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_queue;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;

		HANDLE m_fence_event = nullptr;
		uint64_t m_next_fence_value = 1;
	};
}
