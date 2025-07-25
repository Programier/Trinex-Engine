#pragma once
#include <Core/enums.hpp>
#include <Core/etl/function.hpp>
#include <Core/filesystem/path.hpp>

namespace Engine::VFS
{
	class DirectoryIteratorInterface;
	class File;

	class ENGINE_EXPORT FileSystem
	{
	public:
		enum Type
		{
			Undefined = 0,
			Native    = 1,
			Virtual   = 2,
		};

	private:
		Path m_mount_point;

	public:
		FORCE_INLINE FileSystem(const Path& mount_point = "") : m_mount_point(mount_point) {}
		FORCE_INLINE virtual ~FileSystem() = default;

		trinex_non_moveable(FileSystem);
		trinex_non_copyable(FileSystem);

		FORCE_INLINE const Path& mount_point() const { return m_mount_point; }

		virtual DirectoryIteratorInterface* create_directory_iterator(const Path& path)           = 0;
		virtual DirectoryIteratorInterface* create_recursive_directory_iterator(const Path& path) = 0;
		virtual const Path& path() const                                                          = 0;
		virtual bool is_read_only() const                                                         = 0;
		virtual File* open(const Path& path, FileOpenMode mode)                                   = 0;
		virtual bool create_dir(const Path& path)                                                 = 0;
		virtual bool remove(const Path& path)                                                     = 0;
		virtual bool copy(const Path& src, const Path& dest)                                      = 0;
		virtual bool rename(const Path& src, const Path& dest)                                    = 0;
		virtual bool is_file_exist(const Path& path) const                                        = 0;
		virtual bool is_file(const Path& file) const                                              = 0;
		virtual bool is_dir(const Path& dir) const                                                = 0;
		virtual Type type() const                                                                 = 0;
		virtual Path native_path(const Path& path) const                                          = 0;

		friend class RootFS;
	};
}// namespace Engine::VFS
