#include <Core/exception.hpp>
#include <Core/filesystem/directory_iterator.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/logger.hpp>
#include <android_file.hpp>
#include <android_file_system.hpp>
#include <cerrno>
#include <cstring>
#include <filesystem>

namespace Engine::VFS
{
	namespace fs = std::filesystem;

	template<typename Iterator>
	struct AndroidIterator : public DirectoryIteratorInterface {
		AndroidFileSystem* m_base;
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
			AndroidIterator* new_iterator = trx_new AndroidIterator();
			new_iterator->m_path          = m_path;
			new_iterator->m_it            = m_it;
			new_iterator->m_base          = m_base;
			return new_iterator;
		}

		Identifier id() const override
		{
			static const byte value = 0;
			return reinterpret_cast<Identifier>(&value);
		}

		bool is_equal(DirectoryIteratorInterface* other) override
		{
			return m_it == reinterpret_cast<AndroidIterator*>(other)->m_it;
		}
	};

	DirectoryIteratorInterface* AndroidFileSystem::create_directory_iterator(const Path& path)
	{
		try
		{
			Path dir      = m_directory / path;
			auto iterator = fs::directory_iterator(dir.str());

			AndroidIterator<fs::directory_iterator>* it = trx_new AndroidIterator<fs::directory_iterator>();
			it->m_base                                  = this;
			it->m_it                                    = iterator;
			it->update_path();
			return it;
		}
		catch (const std::exception& e)
		{
			error_log("AndroidFileSystem", "%s", e.what());
			return nullptr;
		}
	}

	DirectoryIteratorInterface* AndroidFileSystem::create_recursive_directory_iterator(const Path& path)
	{
		try
		{
			Path dir      = m_directory / path;
			auto iterator = fs::recursive_directory_iterator(dir.str());

			AndroidIterator<fs::recursive_directory_iterator>* it = trx_new AndroidIterator<fs::recursive_directory_iterator>();
			it->m_base                                            = this;
			it->m_it                                              = iterator;
			it->update_path();
			return it;
		}
		catch (...)
		{
			return nullptr;
		}
	}

	AndroidFileSystem::AndroidFileSystem(const Path& mount, const Path& directory) : FileSystem(mount), m_directory(directory) {}

	const Path& AndroidFileSystem::path() const
	{
		return m_directory;
	}

	bool AndroidFileSystem::is_read_only() const
	{
		return (fs::status(m_directory.str()).permissions() & fs::perms::owner_write) != fs::perms::owner_write;
	}

	File* AndroidFileSystem::open(const Path& path, FileOpenMode mode)
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
			return trx_new AndroidFile(this, full_path, std::move(file));
		}

		error_log("Android FS", "%s: %s", path.c_str(), std::strerror(errno));
		return nullptr;
	}

	AndroidFileSystem& AndroidFileSystem::close(File* file)
	{
		trx_delete file;
		return *this;
	}

	bool AndroidFileSystem::create_dir(const Path& path)
	{
		return fs::create_directories((m_directory / path).str());
	}

	bool AndroidFileSystem::remove(const Path& path)
	{
		return fs::remove((m_directory / path).str());
	}

	bool AndroidFileSystem::copy(const Path& src, const Path& dest)
	{
		std::error_code code;
		fs::copy_file((m_directory / src).str(), (m_directory / dest).str(), code);

		if (code)
		{
			error_log("AndroidFS", "%s", code.message().c_str());
			return false;
		}
		return true;
	}

	bool AndroidFileSystem::rename(const Path& src, const Path& dest)
	{
		std::error_code code;
		fs::rename((m_directory / src).str(), (m_directory / dest).str(), code);

		if (code)
		{
			error_log("AndroidFS", "%s", code.message().c_str());
			return false;
		}
		return true;
	}

	bool AndroidFileSystem::is_file_exist(const Path& path) const
	{
		return fs::exists((m_directory / path).str());
	}

	bool AndroidFileSystem::is_file(const Path& file) const
	{
		return fs::is_regular_file((m_directory / file).str());
	}

	bool AndroidFileSystem::is_dir(const Path& dir) const
	{
		return fs::is_directory((m_directory / dir).str());
	}

	AndroidFileSystem::Type AndroidFileSystem::type() const
	{
		return Type::Native;
	}

	Path AndroidFileSystem::native_path(const Path& path) const
	{
		return m_directory / path;
	}
}// namespace Engine::VFS


namespace Engine::Platform::FileSystem
{
	ENGINE_EXPORT VFS::FileSystem* create(const Path& mount, const Path& path)
	{
		return trx_new VFS::AndroidFileSystem(mount, path);
	}

	ENGINE_EXPORT void destroy(VFS::FileSystem* fs)
	{
		trx_delete_inline(fs);
	}
}// namespace Engine::Platform::FileSystem
