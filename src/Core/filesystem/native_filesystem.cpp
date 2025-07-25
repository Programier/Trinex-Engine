#include "vfs_log.hpp"
#include <Core/exception.hpp>
#include <Core/filesystem/directory_iterator.hpp>
#include <Core/filesystem/native_file.hpp>
#include <Core/filesystem/native_file_system.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/logger.hpp>
#include <cerrno>
#include <cstring>
#include <filesystem>

namespace Engine::VFS
{
	namespace fs = std::filesystem;

	template<typename Iterator>
	struct NativeIterator : public DirectoryIteratorInterface {
		NativeFileSystem* m_base;
		Iterator m_it;
		Path m_path;

		bool update_path()
		{
			if (is_valid())
			{
				const std::filesystem::path& path = *m_it;
				m_path                            = m_base->mount_point() / Path(path.string()).relative(m_base->path());
				return true;
			}
			return false;
		}

		bool next() override
		{
			++m_it;
			return update_path();
		}

		const Path& path() override { return m_path; }

		bool is_valid() const override
		{
			static Iterator tmp;
			return m_it != tmp;
		}

		DirectoryIteratorInterface* copy() override
		{
			NativeIterator* new_iterator = new NativeIterator();
			new_iterator->m_path         = m_path;
			new_iterator->m_it           = m_it;
			new_iterator->m_base         = m_base;
			return new_iterator;
		}

		Identifier id() const override
		{
			static const byte value = 0;
			return reinterpret_cast<Identifier>(&value);
		}

		bool is_equal(DirectoryIteratorInterface* other) override
		{
			return m_it == reinterpret_cast<NativeIterator*>(other)->m_it;
		}
	};

	DirectoryIteratorInterface* NativeFileSystem::create_directory_iterator(const Path& path)
	{
		try
		{
			Path dir      = m_directory / path;
			auto iterator = fs::directory_iterator(dir.str());

			NativeIterator<fs::directory_iterator>* it = new NativeIterator<fs::directory_iterator>();
			it->m_base                                 = this;
			it->m_it                                   = iterator;
			it->update_path();
			return it;
		}
		catch (const std::exception& e)
		{
			error_log("NativeFileSystem", "%s", e.what());
			return nullptr;
		}
	}

	DirectoryIteratorInterface* NativeFileSystem::create_recursive_directory_iterator(const Path& path)
	{
		try
		{
			Path dir      = m_directory / path;
			auto iterator = fs::recursive_directory_iterator(dir.str());

			NativeIterator<fs::recursive_directory_iterator>* it = new NativeIterator<fs::recursive_directory_iterator>();
			it->m_base                                           = this;
			it->m_it                                             = iterator;
			it->update_path();
			return it;
		}
		catch (...)
		{
			return nullptr;
		}
	}


	NativeFileSystem::NativeFileSystem(const Path& mount_point, const Path& directory)
	    : FileSystem(mount_point), m_directory(directory)
	{}

	const Path& NativeFileSystem::path() const
	{
		return m_directory;
	}

	bool NativeFileSystem::is_read_only() const
	{
		return (fs::status(m_directory.str()).permissions() & fs::perms::owner_write) != fs::perms::owner_write;
	}

	File* NativeFileSystem::open(const Path& path, FileOpenMode mode)
	{
		if (is_dir(path))
			return nullptr;

		Path full_path = m_directory / path;

		std::ios_base::openmode open_mode = std::ios_base::binary;

		bool is_read_only = (mode & (FileOpenMode::Out | FileOpenMode::Append)) == 0;

		if (mode & FileOpenMode::In)
			open_mode |= std::ios_base::in;
		if (mode & FileOpenMode::Out)
			open_mode |= std::ios_base::out;
		if (mode & FileOpenMode::Append)
			open_mode |= std::ios_base::app;
		if (mode & FileOpenMode::Trunc)
			open_mode |= std::ios_base::trunc;

		std::fstream file;
		file.open(full_path.str(), open_mode);
		if (file.is_open())
		{
			return new NativeFile(path, full_path, std::move(file), is_read_only);
		}
		else
		{
			error_log("Native FS", "%s: %s", path.c_str(), std::strerror(errno));
		}
		return nullptr;
	}

	bool NativeFileSystem::create_dir(const Path& path)
	{
		return fs::create_directories((m_directory / path).str());
	}

	bool NativeFileSystem::remove(const Path& path)
	{
		return fs::remove((m_directory / path).str());
	}

	bool NativeFileSystem::copy(const Path& src, const Path& dest)
	{
		std::error_code code;
		fs::copy_file((m_directory / src).str(), (m_directory / dest).str(), code);

		if (code)
		{
			vfs_error("%s", code.message().c_str());
			return false;
		}
		return true;
	}

	bool NativeFileSystem::rename(const Path& src, const Path& dest)
	{
		std::error_code code;
		fs::rename((m_directory / src).str(), (m_directory / dest).str(), code);

		if (code)
		{
			vfs_error("%s", code.message().c_str());
			return false;
		}
		return true;
	}

	bool NativeFileSystem::is_file_exist(const Path& path) const
	{
		return fs::exists((m_directory / path).str());
	}

	bool NativeFileSystem::is_file(const Path& file) const
	{
		return fs::is_regular_file((m_directory / file).str());
	}

	bool NativeFileSystem::is_dir(const Path& dir) const
	{
		return fs::is_directory((m_directory / dir).str());
	}

	NativeFileSystem::Type NativeFileSystem::type() const
	{
		return Type::Native;
	}

	Path NativeFileSystem::native_path(const Path& path) const
	{
		return m_directory / path;
	}
}// namespace Engine::VFS
