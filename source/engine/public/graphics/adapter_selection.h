#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <cstdint>
#include <string>
#include <vector>

namespace buz
{
	//--------------------------------------------------------------
	/// @brief Which graphics card the engine should pick from the scored list.
	enum class adapter_preference
	{
		highest_score, // the most capable graphics card (default)
		lowest_score,  // the least capable graphics card, useful to test low-end behavior
	};

	//--------------------------------------------------------------
	/// @brief Information and capability score of a single graphics adapter.
	struct adapter_info
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

		std::string description;
		uint64_t dedicated_video_memory = 0;
		bool is_software = false;

		D3D_FEATURE_LEVEL max_feature_level = D3D_FEATURE_LEVEL_11_0;
		D3D_SHADER_MODEL highest_shader_model = D3D_SHADER_MODEL_6_0;

		uint64_t score = 0;
	};

	//--------------------------------------------------------------
	/// @brief Enumerates all directx 12 capable adapters on this machine and scores them.
	///
	/// Similar to how unreal picks its gpu: every adapter gets a score based
	/// on its dedicated video memory, maximum feature level and highest
	/// supported shader model. Software adapters (warp) are listed but score
	/// zero, so they are only picked when no real graphics card is present.
	///
	/// @param factory Factory used to enumerate the adapters.
	/// @return All directx 12 capable adapters, in enumeration order.
	std::vector<adapter_info> enumerate_and_score_adapters(IDXGIFactory1* factory);

	//--------------------------------------------------------------
	/// @brief Selects an adapter from a scored list based on the given preference.
	/// @param adapters Scored adapter list from enumerate_and_score_adapters.
	/// @param preference Whether to pick the highest or lowest scoring adapter.
	/// @return Selected adapter, or nullptr when the list is empty.
	const adapter_info* select_adapter(const std::vector<adapter_info>& adapters, adapter_preference preference);
}
