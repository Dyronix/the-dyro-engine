#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <string>

namespace dyro
{
	//--------------------------------------------------------------
	/// @brief A fixed-size win32 window the engine renders into.
	class window
	{
	public:
		//----------------------------------------------------------
		/// @brief Registers the window class and creates the window.
		/// @param width Width of the drawable area in pixels.
		/// @param height Height of the drawable area in pixels.
		/// @param title Text shown in the title bar.
		/// @return True when the window was created successfully.
		bool initialize(uint32_t width, uint32_t height, const std::wstring& title);

		//----------------------------------------------------------
		/// @brief Handles all pending window messages.
		/// @return False when the window was closed and the game should stop.
		bool process_messages();

		//----------------------------------------------------------
		/// @brief Returns the native win32 window handle.
		HWND get_handle() const { return m_handle; }

		//----------------------------------------------------------
		/// @brief Returns the width of the drawable area in pixels.
		uint32_t get_width() const { return m_width; }

		//----------------------------------------------------------
		/// @brief Returns the height of the drawable area in pixels.
		uint32_t get_height() const { return m_height; }

	private:
		HWND m_handle = nullptr;

		uint32_t m_width = 0;
		uint32_t m_height = 0;
	};
}
