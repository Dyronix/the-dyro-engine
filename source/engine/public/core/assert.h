#pragma once

#include <format>
#include <string_view>
#include <utility>

// Asserts are compiled in for debug builds only, exactly like logging. In
// release (NDEBUG) a DYX_ASSERT disappears completely: the condition is not
// even evaluated, so never put code with side effects inside one.
#if !defined(NDEBUG)
	#define DYX_ASSERT_ENABLED
#endif

namespace dyx
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
	/// debug and release builds (std::format "{}" style formatting).
	/// @param format Format string with "{}" placeholders, checked at compile time.
	/// @param args Values inserted into the placeholders.
	template<typename... Args>
	[[noreturn]] void fatal_error(std::format_string<Args...> format, Args&&... args)
	{
		internal::on_fatal_error(std::format(format, std::forward<Args>(args)...));
	}
}

//--------------------------------------------------------------
/// @brief Halts the program in the debugger when the condition is false.
///
/// Use it to document assumptions your code makes:
/// @code{.cpp}
/// DYX_ASSERT(frame_index < frame_count);
/// @endcode
/// Compiled away in release builds.
#if defined(DYX_ASSERT_ENABLED)
	#define DYX_ASSERT(condition) \
		do \
		{ \
			if (!(condition)) \
			{ \
				dyx::internal::on_assert_failed(#condition, __FILE__, __LINE__, {}); \
				__debugbreak(); \
			} \
		} while (false)
#else
	#define DYX_ASSERT(condition) ((void)0)
#endif

//--------------------------------------------------------------
/// @brief Same as DYX_ASSERT, with an extra message explaining the failure
/// (std::format "{}" style formatting):
/// @code{.cpp}
/// DYX_ASSERT_MSG(frame_index < frame_count, "frame {} out of range", frame_index);
/// @endcode
#if defined(DYX_ASSERT_ENABLED)
	#define DYX_ASSERT_MSG(condition, ...) \
		do \
		{ \
			if (!(condition)) \
			{ \
				dyx::internal::on_assert_failed(#condition, __FILE__, __LINE__, std::format(__VA_ARGS__)); \
				__debugbreak(); \
			} \
		} while (false)
#else
	#define DYX_ASSERT_MSG(condition, ...) ((void)0)
#endif
