#include <Core/blob.hpp>
#include <Core/filesystem/file.hpp>
#include <Core/filesystem/root_filesystem.hpp>

namespace Trinex
{
	ENGINE_EXPORT VFS::RootFS* rootfs()
	{
		return VFS::RootFS::instance();
	}

}// namespace Trinex

namespace Trinex::VFS
{
	namespace
	{
		class Redirector : public FileSystem
		{
		public:
			Path m_path;

		public:
			Redirector(const PathView& path) : m_path(path) {}

			Ref<File> open(PathView path, AccessFlags flags) override { return rootfs()->open(m_path / path, flags); }
			Ref<Blob> map(PathView path, AccessFlags flags) override { return rootfs()->map(m_path / path, flags); }
			bool stat(PathView path, FileStat& out) const override { return rootfs()->stat(m_path / path, out); }

			bool create_directory(PathView path) override { return rootfs()->create_directory(m_path / path); }
			bool remove(PathView path) override { return rootfs()->remove(m_path / path); }
			bool copy(PathView src, PathView dst) override { return rootfs()->copy(m_path / src, m_path / dst); }
			bool move(PathView src, PathView dst) override { return rootfs()->move(m_path / src, m_path / dst); }

			bool walk(PathView path, const WalkCallback& callback, WalkFlags flags = WalkFlags::Default) const override
			{
				return rootfs()->walk(m_path / path, callback, flags);
			}
		};
	}// namespace

	RootFS* RootFS::s_instance = nullptr;

	Ref<File> RootFS::open(PathView path, AccessFlags flags)
	{
		if (FileSystem* fs = resolve(path))
		{
			return fs->open(path, flags);
		}

		return nullptr;
	}

	Ref<Blob> RootFS::map(PathView path, AccessFlags flags)
	{
		if (FileSystem* fs = resolve(path))
		{
			return fs->map(path, flags);
		}

		return nullptr;
	}

	bool RootFS::stat(PathView path, FileStat& out) const
	{
		if (FileSystem* fs = resolve(path))
		{
			return fs->stat(path, out);
		}

		return false;
	}

	bool RootFS::create_directory(PathView path)
	{
		if (FileSystem* fs = resolve(path))
		{
			return fs->create_directory(path);
		}

		return false;
	}

	bool RootFS::remove(PathView path)
	{
		if (FileSystem* fs = resolve(path))
		{
			return fs->create_directory(path);
		}

		return false;
	}

	bool RootFS::copy(PathView src, PathView dst)
	{
		return false;
	}

	bool RootFS::move(PathView src, PathView dst)
	{
		return false;
	}

	bool RootFS::walk(PathView path, const WalkCallback& callback, WalkFlags flags) const
	{
		return false;
	}

	bool RootFS::mount(PathView point, PathView path)
	{
		auto ref = Ref<Redirector>::make(path);
		return mount(point, ref.value());
	}

	bool RootFS::mount(PathView point, FileSystem* system)
	{
		if (system == nullptr)
			return false;

		if (m_file_systems.contains(point.path()))
			return false;

		m_file_systems.emplace(point, Ref<FileSystem>::retain(system));
		return true;
	}

	bool RootFS::unmount(PathView point)
	{
		auto it = m_file_systems.find(point);

		if (it == m_file_systems.end())
			return false;

		m_file_systems.erase(it);
		return true;
	}

	FileSystem* RootFS::resolve(PathView& path) const
	{
		return nullptr;
	}
}// namespace Trinex::VFS
