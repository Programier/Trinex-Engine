#pragma once
#include <filesystem>

namespace Reflector
{
	class Module;

	bool generate_header(std::filesystem::path path, const Module* module);
	bool generate_source(std::filesystem::path path, const Module* module);
}// namespace Reflector
