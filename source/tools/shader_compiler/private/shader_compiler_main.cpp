//--------------------------------------------------------------
// shader_compiler
//
// Compiles a single hlsl source file to directx bytecode (a *.cso
// file) using the directx shader compiler (dxc). Directx 12 can only
// load compiled shaders, so this tool runs as part of the build:
// the build system invokes it for every shader that changed since
// the last build (see BUZ_COMPILE_SHADERS in the root CMakeLists).
//
// Usage:
//     shader_compiler <input.hlsl> <output.cso> <profile> [entry_point]
//
// Example:
//     shader_compiler sprite_vs.hlsl sprite_vs.cso vs_6_0 main
//--------------------------------------------------------------

// dxcapi.h needs the full com headers, so no WIN32_LEAN_AND_MEAN here
#include <windows.h>

#include <dxcapi.h>
#include <wrl/client.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace
{
	//--------------------------------------------------------------
	/// @brief Converts a regular string to the wide string dxc expects.
	std::wstring to_wide(const std::string& text)
	{
		return std::wstring(text.begin(), text.end());
	}

	//--------------------------------------------------------------
	/// @brief Reads a whole file into a byte buffer.
	/// @return True when the file could be read.
	bool read_file(const std::filesystem::path& path, std::vector<char>& out_data)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			return false;
		}

		const std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		out_data.resize(static_cast<size_t>(size));
		file.read(out_data.data(), size);

		return true;
	}

	//--------------------------------------------------------------
	/// @brief Writes a byte buffer to a file, creating directories when needed.
	/// @return True when the file could be written.
	bool write_file(const std::filesystem::path& path, const void* data, size_t size)
	{
		std::error_code error;
		std::filesystem::create_directories(path.parent_path(), error);

		std::ofstream file(path, std::ios::binary);
		if (!file.is_open())
		{
			return false;
		}

		file.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
		return true;
	}
}

//--------------------------------------------------------------
int main(int argument_count, char* arguments[])
{
	if (argument_count < 4)
	{
		std::printf("Usage: shader_compiler <input.hlsl> <output.cso> <profile> [entry_point]\n");
		std::printf("Example: shader_compiler sprite_vs.hlsl sprite_vs.cso vs_6_0 main\n");
		return 1;
	}

	const std::filesystem::path input_path = arguments[1];
	const std::filesystem::path output_path = arguments[2];
	const std::wstring profile = to_wide(arguments[3]);
	const std::wstring entry_point = to_wide(argument_count > 4 ? arguments[4] : "main");

	std::vector<char> source;
	if (!read_file(input_path, source))
	{
		std::fprintf(stderr, "shader_compiler: cannot open input file \"%s\"\n", input_path.string().c_str());
		return 1;
	}

	// Create the dxc compiler
	ComPtr<IDxcCompiler3> compiler;
	if (FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler))))
	{
		std::fprintf(stderr, "shader_compiler: failed to create the dxc compiler\n");
		return 1;
	}

	const std::wstring input_name = input_path.filename().wstring();

	// Command line arguments for dxc, same as you would pass to dxc.exe
	const wchar_t* compile_arguments[] =
	{
		input_name.c_str(),          // shown in error messages
		L"-T", profile.c_str(),      // target profile (vs_6_0, ps_6_0, ...)
		L"-E", entry_point.c_str(),  // entry point function
		L"-O3",                      // full optimization
		L"-WX",                      // treat warnings as errors
	};

	DxcBuffer source_buffer = {};
	source_buffer.Ptr = source.data();
	source_buffer.Size = source.size();
	source_buffer.Encoding = DXC_CP_UTF8;

	ComPtr<IDxcResult> result;
	if (FAILED(compiler->Compile(&source_buffer, compile_arguments, _countof(compile_arguments), nullptr, IID_PPV_ARGS(&result))))
	{
		std::fprintf(stderr, "shader_compiler: the compile call itself failed\n");
		return 1;
	}

	// Print compiler errors and warnings, if any
	ComPtr<IDxcBlobUtf8> errors;
	result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
	if (errors != nullptr && errors->GetStringLength() > 0)
	{
		std::fprintf(stderr, "%s", errors->GetStringPointer());
	}

	HRESULT compile_status = S_OK;
	result->GetStatus(&compile_status);
	if (FAILED(compile_status))
	{
		std::fprintf(stderr, "shader_compiler: failed to compile \"%s\"\n", input_path.string().c_str());
		return 1;
	}

	// Write the compiled bytecode to the output file
	ComPtr<IDxcBlob> bytecode;
	if (FAILED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&bytecode), nullptr)))
	{
		std::fprintf(stderr, "shader_compiler: no bytecode was produced\n");
		return 1;
	}

	if (!write_file(output_path, bytecode->GetBufferPointer(), bytecode->GetBufferSize()))
	{
		std::fprintf(stderr, "shader_compiler: cannot write output file \"%s\"\n", output_path.string().c_str());
		return 1;
	}

	std::printf("shader_compiler: %s -> %s (%zu bytes)\n", input_path.string().c_str(), output_path.string().c_str(), bytecode->GetBufferSize());
	return 0;
}
