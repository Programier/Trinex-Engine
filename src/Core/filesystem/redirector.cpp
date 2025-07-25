#include <Core/filesystem/redirector.hpp>
#include <Core/filesystem/root_filesystem.hpp>

namespace Engine::VFS
{
	Redirector::Redirector(const Path& mount_point, const Path& redirect_path)
	    : FileSystem(mount_point), m_redirect(redirect_path)
	{}

	DirectoryIteratorInterface* Redirector::create_directory_iterator(const Path& path)
	{
		FileSystem* fs = rootfs()->filesystem_of(m_redirect);
		return fs->create_directory_iterator(m_redirect.relative(fs->mount_point()) / path);
	}

	DirectoryIteratorInterface* Redirector::create_recursive_directory_iterator(const Path& path)
	{
		FileSystem* fs = rootfs()->filesystem_of(m_redirect);
		return fs->create_recursive_directory_iterator(m_redirect.relative(fs->mount_point()) / path);
	}

	const Path& Redirector::path() const
	{
		FileSystem* fs = rootfs()->filesystem_of(m_redirect);
		m_path         = fs->path();

		if (fs->type() == Native)
			m_path /= m_redirect.relative(fs->mount_point());

		return m_path;
	}

	bool Redirector::is_read_only() const
	{
		return rootfs()->filesystem_of(m_redirect);
	}

	File* Redirector::open(const Path& path, FileOpenMode mode)
	{
		FileSystem* fs = rootfs()->filesystem_of(m_redirect);
		return fs->open(m_redirect.relative(fs->mount_point()) / path, mode);
	}

	bool Redirector::create_dir(const Path& path)
	{
		FileSystem* fs = rootfs()->filesystem_of(m_redirect);
		return fs->create_dir(m_redirect.relative(fs->mount_point()) / path);
	}

	bool Redirector::remove(const Path& path)
	{
		FileSystem* fs = rootfs()->filesystem_of(m_redirect);
		return fs->remove(m_redirect.relative(fs->mount_point()) / path);
	}

	bool Redirector::copy(const Path& src, const Path& dest)
	{
		return false;
	}

	bool Redirector::rename(const Path& src, const Path& dest)
	{
		return false;
	}

	bool Redirector::is_file_exist(const Path& path) const
	{
		return false;
	}

	bool Redirector::is_file(const Path& file) const
	{
		return false;
	}

	bool Redirector::is_dir(const Path& dir) const
	{
		return false;
	}

	Redirector::Type Redirector::type() const
	{
		return rootfs()->filesystem_of(m_redirect)->type();
	}

	Path Redirector::native_path(const Path& path) const
	{
		FileSystem* fs = rootfs()->filesystem_of(m_redirect);
		return fs->native_path(m_redirect.relative(fs->mount_point()) / path);
	}
}// namespace Engine::VFS
