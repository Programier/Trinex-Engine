#include <Core/exception.hpp>
#include <Core/filesystem/directory_iterator.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/logger.hpp>
#include <cerrno>
#include <common_file.hpp>
#include <common_file_system.hpp>
#include <cstring>
#include <filesystem>

namespace Engine::VFS
{
	namespace fs = std::filesystem;

	template<typename Iterator>
	struct CommonIterator : public DirectoryIteratorInterface {
		CommonFileSystem* m_base;
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
			CommonIterator* new_iterator = trx_new CommonIterator();
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
			return m_it == reinterpret_cast<CommonIterator*>(other)->m_it;
		}
	};

	DirectoryIteratorInterface* CommonFileSystem::create_directory_iterator(const Path& path)
	{
		try
		{
			Path dir      = m_directory / path;
			auto iterator = fs::directory_iterator(dir.str());

			CommonIterator<fs::directory_iterator>* it = trx_new CommonIterator<fs::directory_iterator>();
			it->m_base                                 = this;
			it->m_it                                   = iterator;
			it->update_path();
			return it;
		}
		catch (const std::exception& e)
		{
			error_log("CommonFileSystem", "%s", e.what());
			return nullptr;
		}
	}

	DirectoryIteratorInterface* CommonFileSystem::create_recursive_directory_iterator(const Path& path)
	{
		try
		{
			Path dir      = m_directory / path;
			auto iterator = fs::recursive_directory_iterator(dir.str());

			CommonIterator<fs::recursive_directory_iterator>* it = trx_new CommonIterator<fs::recursive_directory_iterator>();
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

	CommonFileSystem::CommonFileSystem(const Path& mount, const Path& directory) : FileSystem(mount), m_directory(directory) {}

	const Path& CommonFileSystem::path() const
	{
		return m_directory;
	}

	bool CommonFileSystem::is_read_only() const
	{
		return (fs::status(m_directory.str()).permissions() & fs::perms::owner_write) != fs::perms::owner_write;
	}

	File* CommonFileSystem::open(const Path& path, FileOpenMode mode)
	{
		if (is_dir(path))
			return nullptr;

		Path full_path = m_directory / path;

		std::ios_base::openmode open_mode = std::ios_base::binary;

		if (mode & FileOpenMode::In)
			open_mode |= std::ios_base::in;
		if (mode & FileOpenMode::Out)
			open_mode |= std::ios_base::out;
		if (mode & FileOpenMode::Append)
			open_mode |= std::ios_base::app;

		std::fstream file;
		file.open(full_path.str(), open_mode);

		if (file.is_open())
		{
			if (mode & FileOpenMode::In)
			{
				if (mode & FileOpenMode::Out)
					return trx_new CommonFile(this, full_path, std::move(file));

				return trx_new ReadOnlyCommonFile(this, full_path, std::move(file));
			}

			return trx_new WriteOnlyCommonFile(this, full_path, std::move(file));
		}

		error_log("Common FS", "%s: %s", path.c_str(), std::strerror(errno));
		return nullptr;
	}

	CommonFileSystem& CommonFileSystem::close(File* file)
	{
		trx_delete file;
		return *this;
	}

	bool CommonFileSystem::create_dir(const Path& path)
	{
		return fs::create_directories((m_directory / path).str());
	}

	bool CommonFileSystem::remove(const Path& path)
	{
		return fs::remove((m_directory / path).str());
	}

	bool CommonFileSystem::copy(const Path& src, const Path& dest)
	{
		std::error_code code;
		fs::copy_file((m_directory / src).str(), (m_directory / dest).str(), code);

		if (code)
		{
			error_log("CommonFS", "%s", code.message().c_str());
			return false;
		}
		return true;
	}

	bool CommonFileSystem::rename(const Path& src, const Path& dest)
	{
		std::error_code code;
		fs::rename((m_directory / src).str(), (m_directory / dest).str(), code);

		if (code)
		{
			error_log("CommonFS", "%s", code.message().c_str());
			return false;
		}
		return true;
	}

	bool CommonFileSystem::is_file_exist(const Path& path) const
	{
		return fs::exists((m_directory / path).str());
	}

	bool CommonFileSystem::is_file(const Path& file) const
	{
		return fs::is_regular_file((m_directory / file).str());
	}

	bool CommonFileSystem::is_dir(const Path& dir) const
	{
		return fs::is_directory((m_directory / dir).str());
	}

	CommonFileSystem::Type CommonFileSystem::type() const
	{
		return Type::Native;
	}

	Path CommonFileSystem::native_path(const Path& path) const
	{
		return m_directory / path;
	}
}// namespace Engine::VFS


namespace Engine::Platform::FileSystem
{
	ENGINE_EXPORT VFS::FileSystem* create(const Path& mount, const Path& path)
	{
		return trx_new VFS::CommonFileSystem(mount, path);
	}

	ENGINE_EXPORT void destroy(VFS::FileSystem* fs)
	{
		trx_delete_inline(fs);
	}
}// namespace Engine::Platform::FileSystem
