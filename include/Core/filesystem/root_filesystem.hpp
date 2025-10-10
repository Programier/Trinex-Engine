#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/filesystem/filesystem.hpp>

namespace Engine::VFS
{
	class ENGINE_EXPORT RootFS : public Singletone<RootFS, FileSystem>
	{
	public:
		using FileSystems = TreeMap<String, FileSystem*, std::greater<String>>;

	private:
		FileSystems m_file_systems;
		FileSystem* m_root_native_file_system;

		static RootFS* s_instance;

	private:
		RootFS();
		~RootFS();

	protected:
		DirectoryIteratorInterface* create_directory_iterator(const Path& path) override;
		DirectoryIteratorInterface* create_recursive_directory_iterator(const Path& path) override;

	public:
		bool mount(const Path& mount_point, const Path& path);
		bool mount(const Path& mount_point, const Path& path, Type type);
		RootFS& unmount(const Path& mount_point);
		Pair<FileSystem*, Path> find_filesystem(const Path& path) const;

		const Path& path() const override;
		bool is_read_only() const override;
		File* open(const Path& path, FileOpenMode mode) override;
		RootFS& close(File* file) override;
		bool create_dir(const Path& path) override;
		bool remove(const Path& path) override;
		bool copy(const Path& src, const Path& dest) override;
		bool rename(const Path& src, const Path& dest) override;
		bool is_file_exist(const Path& path) const override;
		bool is_file(const Path& file) const override;
		bool is_dir(const Path& dir) const override;
		Type type() const override;
		Path native_path(const Path& path) const override;
		FileSystem* filesystem_of(const Path& path) const;
		bool pack_native_folder(const Path& native, const Path& virtual_fs, const StringView& password = {}) const;
		Vector<String> mount_points() const;
		const FileSystems& filesystems() const;

		friend class Singletone<RootFS, FileSystem>;
		friend class DirectoryIterator;
		friend class RecursiveDirectoryIterator;
	};
}// namespace Engine::VFS


namespace Engine
{
	ENGINE_EXPORT VFS::RootFS* rootfs();
}
