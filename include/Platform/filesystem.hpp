#pragma once
#include <Core/enums.hpp>
#include <Core/etl/vector.hpp>
#include <Core/types/path.hpp>
#include <Platform/enums.hpp>
#include <Platform/object.hpp>
#include <Platform/types.hpp>

namespace Trinex::Platform
{
	struct FileOpenDesc {
		Path path;
		FileAccess access          = FileAccess::Read;
		FileShare share            = FileShare::Read;
		FileCreateMode create_mode = FileCreateMode::OpenExisting;
	};

	struct FileInfo {
		Path path;
		FileAttribute attributes = FileAttribute::Undefined;
		u64 size                 = 0;
		u64 created_time         = 0;
		u64 modified_time        = 0;
		u64 accessed_time        = 0;
	};

	class ENGINE_EXPORT File : public Object
	{
	public:
		virtual FilePosition rseek(FileOffset offset, IOWhence dir) = 0;
		virtual FilePosition rpos()                                 = 0;
		virtual FilePosition wseek(FileOffset offset, IOWhence dir) = 0;
		virtual FilePosition wpos()                                 = 0;
		virtual usize read(void* buffer, usize size)                = 0;
		virtual usize write(const void* buffer, usize size)         = 0;
		virtual bool flush()                                        = 0;
		virtual u64 size() const                                    = 0;
	};

	class ENGINE_EXPORT DirectoryIterator : public Object
	{
	public:
		virtual bool next(FileInfo* out) = 0;
	};

	class ENGINE_EXPORT FileSystem
	{
	public:
		static FileSystem* instance();

		virtual ~FileSystem() = default;

		virtual File* open(const FileOpenDesc* desc)                                     = 0;
		virtual DirectoryIterator* create_directory_iterator(const Path* path)           = 0;
		virtual DirectoryIterator* create_recursive_directory_iterator(const Path* path) = 0;
		virtual bool stat(const Path* path, FileInfo* out) const                         = 0;
		virtual bool exists(const Path* path) const                                      = 0;
		virtual bool is_file(const Path* path) const                                     = 0;
		virtual bool is_dir(const Path* path) const                                      = 0;
		virtual bool create_dir(const Path* path, bool recursive = false)                = 0;
		virtual bool remove(const Path* path, bool recursive = false)                    = 0;
		virtual bool copy(const Path* src, const Path* dst, bool overwrite = false)      = 0;
		virtual bool rename(const Path* src, const Path* dst, bool overwrite = false)    = 0;
		virtual Path absolute_path(const Path* path) const                               = 0;
		virtual Path canonical_path(const Path* path) const                              = 0;
		virtual Path temp_directory() const                                              = 0;
		virtual Path user_directory() const                                              = 0;
	};
}// namespace Trinex::Platform
