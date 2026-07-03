#include "application/window.h"

#include "core/log.h"

namespace
{
	constexpr const wchar_t* window_class_name = L"dyro_engine_window";

	//--------------------------------------------------------------
	/// @brief Handles messages windows sends to our window.
	LRESULT CALLBACK window_procedure(HWND handle, UINT message, WPARAM w_param, LPARAM l_param)
	{
		switch (message)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		default:
			return DefWindowProcW(handle, message, w_param, l_param);
		}
	}
}

namespace dyro
{
	//--------------------------------------------------------------
	bool window::initialize(uint32_t width, uint32_t height, const std::wstring& title)
	{
		m_width = width;
		m_height = height;

		const HINSTANCE instance = GetModuleHandleW(nullptr);

		WNDCLASSEXW window_class = {};
		window_class.cbSize = sizeof(window_class);
		window_class.lpfnWndProc = window_procedure;
		window_class.hInstance = instance;
		window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
		window_class.lpszClassName = window_class_name;

		if (RegisterClassExW(&window_class) == 0)
		{
			log::error("Failed to register the window class");
			return false;
		}

		// Fixed-size window: no resize border and no maximize button, so
		// the swap chain never has to be resized.
		const DWORD style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;

		// The given size is the drawable area; the full window is bigger
		// because of the title bar and borders.
		RECT window_rect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
		AdjustWindowRect(&window_rect, style, FALSE);

		m_handle = CreateWindowExW(
			0,
			window_class_name,
			title.c_str(),
			style,
			CW_USEDEFAULT, CW_USEDEFAULT,
			window_rect.right - window_rect.left,
			window_rect.bottom - window_rect.top,
			nullptr, nullptr, instance, nullptr);

		if (m_handle == nullptr)
		{
			log::error("Failed to create the window");
			return false;
		}

		ShowWindow(m_handle, SW_SHOW);
		return true;
	}

	//--------------------------------------------------------------
	bool window::process_messages()
	{
		MSG message = {};
		while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT)
			{
				return false;
			}

			TranslateMessage(&message);
			DispatchMessageW(&message);
		}

		return true;
	}
}
