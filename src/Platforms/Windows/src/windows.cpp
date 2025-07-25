#include <Core/arguments.hpp>
#include <Core/definitions.hpp>
#include <Core/filesystem/path.hpp>
#include <Platform/platform.hpp>

namespace Engine::Platform
{
	ENGINE_EXPORT OperationSystemType system_type()
	{
		return OperationSystemType::Windows;
	}

	ENGINE_EXPORT const char* system_name()
	{
		return "Windows";
	}

	ENGINE_EXPORT Path find_exec_directory()
	{
		int_t argc        = Arguments::argc();
		const char** argv = Arguments::argv();

		if (argc == 0)// Usually it's impossible, but just in case, let it be
			return Path("./");
		return Path(argv[0]).base_path();
	}
}// namespace Engine::Platform
