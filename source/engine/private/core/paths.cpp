#include "core/paths.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace dyx
{
	namespace paths
	{
		//----------------------------------------------------------
		std::filesystem::path get_executable_directory()
		{
			wchar_t executable_path[MAX_PATH];
			GetModuleFileNameW(nullptr, executable_path, MAX_PATH);

			return std::filesystem::path(executable_path).parent_path();
		}

		//----------------------------------------------------------
		std::filesystem::path get_content_directory()
		{
			return get_executable_directory() / "content";
		}
	}
}
