#include "core/assert.h"

#include "core/log.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdlib>
#include <string>

namespace dyx
{
	namespace internal
	{
		//--------------------------------------------------------------
		void on_assert_failed(const char* condition, std::string_view message, std::source_location location)
		{
			if (message.empty())
			{
				log::error("Assert failed: {} in {} ({}:{})", condition, location.function_name(), location.file_name(), location.line());
			}
			else
			{
				log::error("Assert failed: {} - {} in {} ({}:{})", condition, message, location.function_name(), location.file_name(), location.line());
			}
		}

		//--------------------------------------------------------------
		void on_fatal_error(std::string_view message)
		{
			log::error("Fatal error: {}", message);

			// The message box makes the error impossible to miss, also in
			// release builds where logging is compiled out.
			const std::string text(message);
			MessageBoxA(nullptr, text.c_str(), "Fatal error", MB_OK | MB_ICONERROR);

			std::exit(EXIT_FAILURE);
		}
	}
}
