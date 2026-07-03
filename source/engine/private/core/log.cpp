#include "core/log.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdarg>
#include <cstdio>

namespace
{
	constexpr auto green = "\033[32m";
	constexpr auto magenta = "\033[35m";
	constexpr auto red = "\033[31m";
	constexpr auto reset = "\033[0m";

	//--------------------------------------------------------------
	/// @brief Enables colored output on the windows console (runs once).
	void enable_console_colors()
	{
		static bool enabled = false;
		if (enabled)
		{
			return;
		}

		const HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

		DWORD console_mode = 0;
		if (GetConsoleMode(console_handle, &console_mode))
		{
			SetConsoleMode(console_handle, console_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
		}

		enabled = true;
	}

	//--------------------------------------------------------------
	/// @brief Formats and prints a single log line to the console and the debugger output window.
	void trace(const char* name, const char* color, const char* format, va_list arguments)
	{
		enable_console_colors();

		char message[2048];
		vsnprintf(message, sizeof(message), format, arguments);

		std::printf("%s[%s]%s %s\n", color, name, reset, message);

		// Also forward the message to the visual studio output window
		char debugger_message[2100];
		std::snprintf(debugger_message, sizeof(debugger_message), "[%s] %s\n", name, message);
		OutputDebugStringA(debugger_message);
	}
}

namespace dyro
{
	namespace log
	{
		//----------------------------------------------------------
		void info(const char* format, ...)
		{
			va_list arguments;
			va_start(arguments, format);
			trace("info", green, format, arguments);
			va_end(arguments);
		}

		//----------------------------------------------------------
		void warn(const char* format, ...)
		{
			va_list arguments;
			va_start(arguments, format);
			trace("warn", magenta, format, arguments);
			va_end(arguments);
		}

		//----------------------------------------------------------
		void error(const char* format, ...)
		{
			va_list arguments;
			va_start(arguments, format);
			trace("error", red, format, arguments);
			va_end(arguments);
		}
	}
}
