#include "core/log.h"

#if defined(DYX_LOG_ENABLED)

#include "core/paths.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
#include <fstream>
#include <memory>
#include <vector>

namespace
{
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
	/// @brief Destination a formatted log line is written to.
	struct sink
	{
		virtual ~sink() = default;
		virtual void trace(const char* name, const char* color, std::string_view text) = 0;
	};

	//--------------------------------------------------------------
	/// @brief Writes colored lines to stdout and forwards them to the
	///        visual studio output window.
	struct console_sink : public sink
	{
		void trace(const char* name, const char* color, std::string_view text) override
		{
			enable_console_colors();

			std::printf("%s[%s]%s %.*s\n", color, name, dyx::log::reset,
				static_cast<int>(text.size()), text.data());

			OutputDebugStringA(std::format("[{}] {}\n", name, text).c_str());
		}
	};

	//--------------------------------------------------------------
	/// @brief Appends lines (without color codes) to a log file on disk.
	struct file_sink : public sink
	{
		explicit file_sink(const std::filesystem::path& path)
			: m_file(path, std::ios::trunc)
		{
		}

		void trace(const char* name, const char* /*color*/, std::string_view text) override
		{
			if (!m_file.is_open())
			{
				return;
			}

			m_file << '[' << name << "] " << text << '\n';
			m_file.flush();
		}

	private:
		std::ofstream m_file;
	};

	//--------------------------------------------------------------
	/// @brief Owns the active sinks. Constructed on first use, registering a
	///        console sink and a file sink next to the executable.
	struct log_context
	{
		log_context()
		{
			sinks.emplace_back(std::make_unique<console_sink>());
			sinks.emplace_back(std::make_unique<file_sink>(
				dyx::paths::get_executable_directory() / "dyx.log"));
		}

		std::vector<std::unique_ptr<sink>> sinks;
	};

	//--------------------------------------------------------------
	log_context& context()
	{
		static log_context ctx;
		return ctx;
	}
}

namespace dyx
{
	namespace log
	{
		namespace internal
		{
			//----------------------------------------------------------
			void trace(const char* name, const char* color, std::string_view text)
			{
				for (auto& sink : context().sinks)
				{
					sink->trace(name, color, text);
				}
			}
		}
	}
}

#else

namespace dyx
{
	namespace log
	{
		namespace internal
		{
			//----------------------------------------------------------
			void trace(const char*, const char*, std::string_view)
			{
			}
		}
	}
}

#endif
