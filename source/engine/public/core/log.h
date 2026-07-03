#pragma once

namespace dyro
{
	namespace log
	{
		//----------------------------------------------------------
		/// @brief Logs an informational message (printf style formatting).
		/// @param format Format string, followed by the format arguments.
		void info(const char* format, ...);

		//----------------------------------------------------------
		/// @brief Logs a warning message (printf style formatting).
		/// @param format Format string, followed by the format arguments.
		void warn(const char* format, ...);

		//----------------------------------------------------------
		/// @brief Logs an error message (printf style formatting).
		/// @param format Format string, followed by the format arguments.
		void error(const char* format, ...);
	}
}
