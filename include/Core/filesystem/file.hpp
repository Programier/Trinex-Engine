#pragma once
#include <Core/enums.hpp>
#include <Core/stream.hpp>

namespace Trinex::VFS
{
	class FileSystem;

	class ENGINE_EXPORT File : public Stream
	{
	public:
		virtual FileSystem* filesystem() const = 0;
	};
}// namespace Trinex::VFS
