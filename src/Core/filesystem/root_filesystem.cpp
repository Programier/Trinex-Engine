#include "vfs_log.hpp"
#include <Core/filesystem/directory_iterator.hpp>
#include <Core/filesystem/file.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/filesystem/redirector.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Platform/platform.hpp>

namespace Engine
{
	ENGINE_EXPORT VFS::RootFS* rootfs()
	{
		return VFS::RootFS::instance();
	}

}// namespace Engine

namespace Engine::VFS
{
	class FileSystemIterator : public DirectoryIteratorInterface
	{
	public:
		Vector<DirectoryIteratorInterface*> m_iterators;
		mutable size_t m_index = 0;

		bool next() override
		{
			if (m_index < m_iterators.size())
			{
				if (m_iterators[m_index]->next())
					return true;

				++m_index;
				return is_valid();
			}
			return false;
		}

		const Path& path() override { return m_iterators[m_index]->path(); }

		bool is_valid() const override
		{
			while (m_index < m_iterators.size())
			{
				DirectoryIteratorInterface* it = m_iterators[m_index];
				if (it->is_valid())
					return true;
				++m_index;
			}
			return false;
		}

		DirectoryIteratorInterface* copy() override
		{
			FileSystemIterator* new_iterator = trx_new FileSystemIterator();
			new_iterator->m_iterators.reserve(m_iterators.size() - m_index);

			for (size_t index = m_index, count = m_iterators.size(); index < count; ++index)
				new_iterator->m_iterators.push_back(m_iterators[index]->copy());

			return new_iterator;
		}

		Identifier id() const override { return m_iterators[m_index]->id(); }

		bool is_equal(DirectoryIteratorInterface* interface) override
		{
			if (m_index < m_iterators.size())
			{
				return m_iterators[m_index]->is_equal(interface);
			}
			return false;
		}

		~FileSystemIterator()
		{
			for (auto* it : m_iterators)
			{
				trx_delete_inline(it);
			}
		}
	};

	class MountPointIterator : public DirectoryIteratorInterface
	{
	public:
		Vector<FileSystem*> m_file_systems;
		mutable size_t m_index = 0;

		bool next() override { return ++m_index < m_file_systems.size(); }
		const Path& path() override { return m_file_systems[m_index]->mount_point(); }
		bool is_valid() const override { return m_index < m_file_systems.size(); }

		DirectoryIteratorInterface* copy() override
		{
			MountPointIterator* new_iterator = trx_new MountPointIterator();
			new_iterator->m_file_systems.reserve(m_file_systems.size() - m_index);
			for (size_t index = m_index, count = m_file_systems.size(); index < count; ++index)
				new_iterator->m_file_systems.push_back(m_file_systems[index]);
			return new_iterator;
		}

		Identifier id() const override
		{
			static const byte id = 0;
			return reinterpret_cast<Identifier>(&id);
		}

		bool is_equal(DirectoryIteratorInterface* iterator) override { return false; }
	};

	static void destroy_file_system(FileSystem* fs)
	{
		if (fs->type() == FileSystem::Native)
		{
			Platform::FileSystem::destroy(fs);
		}
		else
		{
			trx_delete fs;
		}
	}

	RootFS* RootFS::s_instance = nullptr;

	RootFS::RootFS()
	{
		m_root_native_file_system = Platform::FileSystem::create("", "");
	}

	RootFS::~RootFS()
	{
		Platform::FileSystem::destroy(m_root_native_file_system);

		for (auto& [mount, fs] : m_file_systems)
		{
			destroy_file_system(fs);
		}
	}

	DirectoryIteratorInterface* RootFS::create_directory_iterator(const Path& path)
	{
		auto entry = find_filesystem(path);
		if (entry.first)
		{
			DirectoryIteratorInterface* iterator = entry.first->create_directory_iterator(entry.second);

			if (entry.second.empty())
			{
				FileSystemIterator* fs_iterator = trx_new FileSystemIterator();
				fs_iterator->m_iterators.push_back(iterator);
				iterator = fs_iterator;

				MountPointIterator* mount_point_iterator = nullptr;

				for (auto& [mount, fs] : filesystems())
				{
					if (mount.starts_with(entry.first->mount_point()) && mount != entry.first->mount_point().str())
					{
						if (mount_point_iterator == nullptr)
							mount_point_iterator = trx_new MountPointIterator();
						mount_point_iterator->m_file_systems.push_back(fs);
					}
				}

				if (mount_point_iterator)
					fs_iterator->m_iterators.push_back(mount_point_iterator);
			}

			return iterator;
		}
		return nullptr;
	}

	DirectoryIteratorInterface* RootFS::create_recursive_directory_iterator(const Path& path)
	{
		auto entry = find_filesystem(path);
		if (entry.first)
		{
			DirectoryIteratorInterface* iterator = entry.first->create_recursive_directory_iterator(entry.second);

			if (entry.second.empty())
			{
				FileSystemIterator* fs_iterator = trx_new FileSystemIterator();
				fs_iterator->m_iterators.push_back(iterator);
				iterator = fs_iterator;

				for (auto& [mount, fs] : filesystems())
				{
					if (mount.starts_with(entry.first->mount_point()) && mount != entry.first->mount_point().str())
						fs_iterator->m_iterators.push_back(fs->create_recursive_directory_iterator(""));
				}
			}

			return iterator;
		}
		return nullptr;
	}

	bool RootFS::mount(const Path& mount_point, const Path& path)
	{
		if (m_file_systems.contains(mount_point))
		{
			vfs_error("Failed to create mount point '%s'. Mount point already exist!", mount_point.c_str());
			return false;
		}

		auto& file_system = m_file_systems[mount_point];
		file_system       = trx_new Redirector(mount_point, path);

		vfs_log("Mounted '%s' to '%s'", file_system->path().c_str(), mount_point.c_str());
		return true;
	}

	bool RootFS::mount(const Path& mount_point, const Path& path, Type type)
	{
		if (m_file_systems.contains(mount_point))
		{
			vfs_error("Failed to create mount point '%s'. Mount point already exist!", mount_point.c_str());
			return false;
		}

		auto& file_system = m_file_systems[mount_point];

		if (type == Native)
			file_system = Platform::FileSystem::create(mount_point, path);

		vfs_log("Mounted '%s' to '%s'", file_system->path().c_str(), mount_point.c_str());
		return true;
	}

	RootFS& RootFS::unmount(const Path& mount_point)
	{
		auto it = m_file_systems.find(mount_point);

		if (it == m_file_systems.end())
			return *this;

		destroy_file_system(it->second);
		m_file_systems.erase(it);
		return *this;
	}

	Pair<FileSystem*, Path> RootFS::find_filesystem(const Path& path) const
	{
		static auto next_symbol_of = [](const Path& path, const String& fs_path) -> char {
			if (fs_path.empty() || fs_path.back() == Path::separator)
				return Path::separator;

			if (path.length() <= fs_path.length())
				return Path::separator;
			return path.str()[fs_path.length()];
		};

		for (auto& [mount_point, fs] : m_file_systems)
		{
			if (path.path().starts_with(mount_point) &&
			    (next_symbol_of(path, mount_point) == Path::separator || mount_point.empty()))
			{
				return {fs, path.relative(mount_point)};
			}
		}

		return {m_root_native_file_system, path};
	}

	const Path& RootFS::path() const
	{
		static const Path p;
		return p;
	}

	bool RootFS::is_read_only() const
	{
		return false;
	}

	File* RootFS::open(const Path& path, FileOpenMode mode)
	{
		auto entry = find_filesystem(path);
		return entry.first->open(entry.second, mode);
	}

	RootFS& RootFS::close(File* file)
	{
		if (file)
		{
			file->filesystem()->close(file);
		}
		return *this;
	}

	bool RootFS::create_dir(const Path& path)
	{
		auto entry = find_filesystem(path);
		if (entry.first)
		{
			return entry.first->create_dir(entry.second);
		}
		return false;
	}

	bool RootFS::remove(const Path& path)
	{
		auto entry = find_filesystem(path);
		if (entry.first)
		{
			return entry.first->remove(entry.second);
		}
		return false;
	}

	bool RootFS::copy(const Path& src, const Path& dest)
	{
		auto entry1 = find_filesystem(src);
		auto entry2 = find_filesystem(dest);

		if (entry1.first && entry1.first == entry2.first)
		{
			return entry1.first->copy(entry1.second, entry2.second);
		}
		return false;
	}

	bool RootFS::rename(const Path& src, const Path& dest)
	{
		auto entry1 = find_filesystem(src);
		auto entry2 = find_filesystem(dest);

		if (entry1.first && entry1.first == entry2.first)
		{
			return entry1.first->rename(entry1.second, entry2.second);
		}
		return false;
	}

	bool RootFS::is_file_exist(const Path& path) const
	{
		auto entry = find_filesystem(path);
		if (entry.first)
		{
			return entry.first->is_file_exist(entry.second);
		}
		return false;
	}

	bool RootFS::is_file(const Path& file) const
	{
		auto entry = find_filesystem(file);
		if (entry.first)
		{
			return entry.first->is_file(entry.second);
		}
		return false;
	}

	bool RootFS::is_dir(const Path& dir) const
	{
		auto entry = find_filesystem(dir);
		if (entry.first)
		{
			return entry.first->is_dir(entry.second);
		}
		return false;
	}

	RootFS::Type RootFS::type() const
	{
		return Type::Native;
	}

	Path RootFS::native_path(const Path& path) const
	{
		auto entry = find_filesystem(path);
		if (entry.first)
		{
			return entry.first->native_path(entry.second);
		}
		return {};
	}

	FileSystem* RootFS::filesystem_of(const Path& path) const
	{
		return find_filesystem(path).first;
	}

	Vector<String> RootFS::mount_points() const
	{
		Vector<String> result;
		result.reserve(m_file_systems.size());

		for (auto& [name, fs] : m_file_systems)
		{
			result.push_back(name);
		}
		return result;
	}

	const RootFS::FileSystems& RootFS::filesystems() const
	{
		return m_file_systems;
	}
}// namespace Engine::VFS
