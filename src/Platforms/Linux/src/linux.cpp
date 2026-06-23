#include <Core/arguments.hpp>
#include <Core/definitions.hpp>
#include <Core/types/path.hpp>
#include <Core/types/uuid.hpp>
#include <Platform/platform.hpp>
#include <errno.h>
#include <sys/random.h>

namespace Trinex::Platform
{
	ENGINE_EXPORT bool create_uuid(UUID& uuid)
	{
		usize offset = 0;
		u8* data     = uuid.data();

		while (offset < UUID::byte_count)
		{
			const ssize_t result = getrandom(data + offset, UUID::byte_count - offset, 0);

			if (result < 0)
			{
				if (errno == EINTR)
					continue;

				uuid = UUID();
				return false;
			}

			offset += static_cast<usize>(result);
		}

		data[6] = static_cast<u8>((data[6] & 0x0F) | 0x40);
		data[8] = static_cast<u8>((data[8] & 0x3F) | 0x80);
		return true;
	}

	ENGINE_EXPORT OperationSystemType system_type()
	{
		return OperationSystemType::Linux;
	}

	ENGINE_EXPORT const char* system_name()
	{
		return "Linux";
	}

	ENGINE_EXPORT Path find_exec_directory()
	{
		i32 argc          = Arguments::argc();
		const char** argv = Arguments::argv();

		if (argc == 0)// Usually it's impossible, but just in case, let it be
			return Path("./");
		return Path(argv[0]).base_path();
	}
}// namespace Trinex::Platform
