#pragma once

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include <string_view>

// Logging is compiled in for debug builds only. In release (NDEBUG) every log
// call collapses to nothing, so no formatting cost is paid and no log file is
// created.
#if !defined(NDEBUG)
	#define DYRO_LOG_ENABLED
#endif

namespace dyro
{
	namespace log
	{
		constexpr auto green = "\033[32m";
		constexpr auto magenta = "\033[35m";
		constexpr auto red = "\033[31m";
		constexpr auto reset = "\033[0m";

		namespace internal
		{
			//------------------------------------------------------------------
			/// @brief Fans a formatted line out to every registered sink.
			void trace(const char* name, const char* color, std::string_view text);
		}

		//----------------------------------------------------------
		/// @brief Logs an informational message (fmt "{}" style formatting).
		/// @param format Format string with "{}" placeholders.
		/// @param args Values inserted into the placeholders.
		template<typename FormatString, typename... Args>
		void info(const FormatString& format, const Args&... args)
		{
#if defined(DYRO_LOG_ENABLED)
			internal::trace("info", green, fmt::format(fmt::runtime(format), args...));
#else
			(void)format;
			((void)args, ...);
#endif
		}

		//----------------------------------------------------------
		/// @brief Logs a warning message (fmt "{}" style formatting).
		/// @param format Format string with "{}" placeholders.
		/// @param args Values inserted into the placeholders.
		template<typename FormatString, typename... Args>
		void warn(const FormatString& format, const Args&... args)
		{
#if defined(DYRO_LOG_ENABLED)
			internal::trace("warn", magenta, fmt::format(fmt::runtime(format), args...));
#else
			(void)format;
			((void)args, ...);
#endif
		}

		//----------------------------------------------------------
		/// @brief Logs an error message (fmt "{}" style formatting).
		/// @param format Format string with "{}" placeholders.
		/// @param args Values inserted into the placeholders.
		template<typename FormatString, typename... Args>
		void error(const FormatString& format, const Args&... args)
		{
#if defined(DYRO_LOG_ENABLED)
			internal::trace("error", red, fmt::format(fmt::runtime(format), args...));
#else
			(void)format;
			((void)args, ...);
#endif
		}
	}
}
