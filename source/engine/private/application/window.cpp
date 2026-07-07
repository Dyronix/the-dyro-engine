#include "application/window.h"

#include "application/input.h"
#include "core/log.h"

namespace
{
	constexpr const wchar_t* window_class_name = L"dyx_engine_window";

	//--------------------------------------------------------------
	/// @brief Handles messages windows sends to our window.
	LRESULT CALLBACK window_procedure(HWND handle, UINT message, WPARAM w_param, LPARAM l_param)
	{
		// The window instance stored itself in the window user data during
		// initialize, so this free function can reach its input state.
		auto* owner = reinterpret_cast<dyx::window*>(GetWindowLongPtrW(handle, GWLP_USERDATA));
		dyx::input* input = owner != nullptr ? owner->get_input() : nullptr;

		switch (message)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_KEYDOWN:
		case WM_KEYUP:
			if (input != nullptr)
			{
				input->on_key_event(static_cast<uint32_t>(w_param), message == WM_KEYDOWN);
			}
			return 0;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			if (input != nullptr)
			{
				input->on_mouse_button_event(dyx::mouse_button::left, message == WM_LBUTTONDOWN);
			}
			return 0;

		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			if (input != nullptr)
			{
				input->on_mouse_button_event(dyx::mouse_button::right, message == WM_RBUTTONDOWN);
			}
			return 0;

		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
			if (input != nullptr)
			{
				input->on_mouse_button_event(dyx::mouse_button::middle, message == WM_MBUTTONDOWN);
			}
			return 0;

		case WM_MOUSEMOVE:
			if (input != nullptr)
			{
				// The position is packed into l_param: x in the low 16 bits,
				// y in the high 16 bits, both relative to the drawable area.
				const auto x = static_cast<short>(LOWORD(l_param));
				const auto y = static_cast<short>(HIWORD(l_param));
				input->on_mouse_move(static_cast<float>(x), static_cast<float>(y));
			}
			return 0;

		case WM_MOUSEWHEEL:
			if (input != nullptr)
			{
				// The wheel reports in multiples of WHEEL_DELTA (120), one
				// "click" of a regular wheel; normalize that to 1.
				const auto raw_delta = static_cast<short>(HIWORD(w_param));
				input->on_mouse_wheel(static_cast<float>(raw_delta) / static_cast<float>(WHEEL_DELTA));
			}
			return 0;

		case WM_KILLFOCUS:
			if (input != nullptr)
			{
				input->on_focus_lost();
			}
			return 0;

		default:
			return DefWindowProcW(handle, message, w_param, l_param);
		}
	}
}

namespace dyx
{
	//--------------------------------------------------------------
	bool window::initialize(uint32_t width, uint32_t height, const std::wstring& title)
	{
		m_width = width;
		m_height = height;

		const HINSTANCE instance = GetModuleHandleW(nullptr);

		const WNDCLASSEXW window_class =
		{
			.cbSize = sizeof(WNDCLASSEXW),
			.lpfnWndProc = window_procedure,
			.hInstance = instance,
			.hCursor = LoadCursor(nullptr, IDC_ARROW),
			.lpszClassName = window_class_name,
		};

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

		// Store this instance on the window, so window_procedure can find it
		// back when messages come in.
		SetWindowLongPtrW(m_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

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
