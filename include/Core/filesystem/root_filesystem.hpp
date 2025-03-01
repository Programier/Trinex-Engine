#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/filesystem/filesystem.hpp>

namespace Engine::VFS
{
	class ENGINE_EXPORT RootFS : public Singletone<RootFS, FileSystem>
	{
	public:
		struct FileSystemCompare {
			bool operator()(const String&, const String& second) const;
		};

		struct FileSystemInfo {
			FileSystem* fs = nullptr;
			String name;
			Path mount;
		};

		using FileSystemMap = TreeMap<String, FileSystemInfo, FileSystemCompare>;

	private:
		FileSystemMap m_file_systems;

		static RootFS* s_instance;

		RootFS();
		~RootFS();

	protected:
		DirectoryIteratorInterface* create_directory_iterator(const Path& path) override;
		DirectoryIteratorInterface* create_recursive_directory_iterator(const Path& path) override;
		FileSystem* remove_fs(const FileSystemMap::iterator& it);

	public:
		bool mount(const Path& mount_point, const StringView& name, FileSystem* system, const UnMountCallback& callback = {});
		FileSystem* unmount(const Path& mount_point);
		Pair<FileSystem*, Path> find_filesystem(const Path& path) const;

		const Path& path() const override;
		bool is_read_only() const override;
		File* open(const Path& path, Flags<FileOpenMode> mode) override;
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
		FileSystem::Type filesystem_type_of(const Path& path) const;
		bool pack_native_folder(const Path& native, const Path& virtual_fs, const StringView& password = {}) const;
		Vector<String> mount_points() const;
		const FileSystemMap& filesystems() const;

		friend class Singletone<RootFS, FileSystem>;
		friend class DirectoryIterator;
		friend class RecursiveDirectoryIterator;
	};
}// namespace Engine::VFS


namespace Engine
{
	ENGINE_EXPORT VFS::RootFS* rootfs();
}
