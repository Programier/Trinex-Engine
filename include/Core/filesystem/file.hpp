#pragma once
#include <Core/enums.hpp>
#include <Core/etl/span.hpp>
#include <Core/stream.hpp>

namespace Trinex
{
	class Blob;
}

namespace Trinex::VFS
{
	class FileSystem;
	struct FileStat;

	class ENGINE_EXPORT File : public Stream
	{
	public:
		virtual FileSystem* filesystem() const                          = 0;
		virtual FileStat stat() const                                   = 0;
		virtual Ref<Blob> map(usize offset = 0, usize size = ~usize(0)) = 0;
	};
}// namespace Trinex::VFS
