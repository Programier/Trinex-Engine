#pragma once
#include <Core/enums.hpp>
#include <Core/etl/function.hpp>
#include <Core/filesystem/types.hpp>
#include <Core/ref_counted.hpp>
#include <Core/types/path.hpp>

namespace Trinex
{
	class Blob;
}

namespace Trinex::VFS
{
	class File;

	class ENGINE_EXPORT FileSystem : public RefCounted
	{
	public:
		using WalkCallback = FunctionRef<WalkResult(PathView path, const FileStat& stat)>;

	public:
		virtual Ref<File> open(PathView path, AccessFlags flags = AccessFlags::Read) = 0;
		virtual Ref<Blob> map(PathView path, AccessFlags flags = AccessFlags::Read)  = 0;
		virtual bool stat(PathView path, FileStat& out) const                        = 0;

		virtual bool create_directory(PathView path)  = 0;
		virtual bool remove(PathView path)            = 0;
		virtual bool copy(PathView src, PathView dst) = 0;
		virtual bool move(PathView src, PathView dst) = 0;

		virtual bool walk(PathView path, const WalkCallback& callback, WalkFlags flags = WalkFlags::Default) const = 0;
		inline bool walk(PathView path, WalkFlags flags, const WalkCallback& callback) { return walk(path, callback, flags); }

		template<typename... Args>
		inline Identifier watch(Args... args)
		{
			return 0;
		}

		inline void unwatch(Identifier id) {}

		friend class RootFS;
	};
}// namespace Trinex::VFS
