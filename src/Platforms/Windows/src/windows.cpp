#include <Core/arguments.hpp>
#include <Core/definitions.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/types/uuid.hpp>
#include <Platform/platform.hpp>
#include <rpc.h>

#ifdef _MSC_VER
#	pragma comment(lib, "Rpcrt4.lib")
#endif

namespace Trinex::Platform
{
	ENGINE_EXPORT bool create_uuid(UUID& uuid)
	{
		GUID guid;

		if (UuidCreate(&guid) != RPC_S_OK)
		{
			uuid = UUID();
			return false;
		}

		static_assert(sizeof(GUID) == UUID::byte_count, "Unexpected GUID size");

		const u8* src = reinterpret_cast<const u8*>(&guid);
		u8* data      = uuid.data();

		for (usize index = 0; index < UUID::byte_count; ++index)
		{
			data[index] = src[index];
		}

		data[6] = static_cast<u8>((data[6] & 0x0F) | 0x40);
		data[8] = static_cast<u8>((data[8] & 0x3F) | 0x80);
		return true;
	}

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
}// namespace Trinex::Platform
