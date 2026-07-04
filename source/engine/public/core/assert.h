#pragma once

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include <string_view>

// Asserts are compiled in for debug builds only, exactly like logging. In
// release (NDEBUG) a DYRO_ASSERT disappears completely: the condition is not
// even evaluated, so never put code with side effects inside one.
#if !defined(NDEBUG)
	#define DYRO_ASSERT_ENABLED
#endif

namespace dyro
{
	namespace internal
	{
		//----------------------------------------------------------
		/// @brief Reports a failed assert (logs it), then returns so the
		/// macro can break into the debugger at the call site.
		void on_assert_failed(const char* condition, const char* file, int line, std::string_view message);

		//----------------------------------------------------------
		/// @brief Reports an unrecoverable error and terminates the program.
		[[noreturn]] void on_fatal_error(std::string_view message);
	}

	//--------------------------------------------------------------
	/// @brief Reports an unrecoverable error and terminates the program:
	/// logs the message, shows a message box, then exits. Works in both
	/// debug and release builds (fmt "{}" style formatting).
	/// @param format Format string with "{}" placeholders.
	/// @param args Values inserted into the placeholders.
	template<typename FormatString, typename... Args>
	[[noreturn]] void fatal_error(const FormatString& format, const Args&... args)
	{
		internal::on_fatal_error(fmt::format(fmt::runtime(format), args...));
	}
}

//--------------------------------------------------------------
/// @brief Halts the program in the debugger when the condition is false.
///
/// Use it to document assumptions your code makes:
/// @code{.cpp}
/// DYRO_ASSERT(frame_index < frame_count);
/// @endcode
/// Compiled away in release builds.
#if defined(DYRO_ASSERT_ENABLED)
	#define DYRO_ASSERT(condition) \
		do \
		{ \
			if (!(condition)) \
			{ \
				dyro::internal::on_assert_failed(#condition, __FILE__, __LINE__, {}); \
				__debugbreak(); \
			} \
		} while (false)
#else
	#define DYRO_ASSERT(condition) ((void)0)
#endif

//--------------------------------------------------------------
/// @brief Same as DYRO_ASSERT, with an extra message explaining the failure
/// (fmt "{}" style formatting):
/// @code{.cpp}
/// DYRO_ASSERT_MSG(frame_index < frame_count, "frame {} out of range", frame_index);
/// @endcode
#if defined(DYRO_ASSERT_ENABLED)
	#define DYRO_ASSERT_MSG(condition, ...) \
		do \
		{ \
			if (!(condition)) \
			{ \
				dyro::internal::on_assert_failed(#condition, __FILE__, __LINE__, fmt::format(__VA_ARGS__)); \
				__debugbreak(); \
			} \
		} while (false)
#else
	#define DYRO_ASSERT_MSG(condition, ...) ((void)0)
#endif
