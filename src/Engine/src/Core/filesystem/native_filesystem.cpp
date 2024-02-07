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
#define vfs_error(...) error_log("VFS", __VA_ARGS__)
    namespace fs = std::filesystem;


    template<typename Iterator>
    struct NativeIterator : public DirectoryIteratorInterface {
        NativeFileSystem* _M_base;
        Iterator _M_it;
        Path _M_path;

        void update_path()
        {
            if (is_valid())
            {
                _M_path = _M_base->mount_point() / Path(fs::relative(*_M_it, _M_base->path().str()));
            }
        }

        void next() override
        {
            ++_M_it;
            update_path();
        }

        const Path& path() override
        {
            return _M_path;
        }

        bool is_valid() const override
        {
            static Iterator tmp;
            return _M_it != tmp;
        }

        DirectoryIteratorInterface* copy() override
        {
            NativeIterator* new_iterator = new NativeIterator();
            new_iterator->_M_path        = _M_path;
            new_iterator->_M_it          = _M_it;
            new_iterator->_M_base        = _M_base;
            return new_iterator;
        }

        Type type() const override
        {
            return Native;
        }

        bool is_equal(DirectoryIteratorInterface* other) override
        {
            return _M_it == reinterpret_cast<NativeIterator*>(other)->_M_it;
        }
    };

    DirectoryIteratorInterface* NativeFileSystem::create_directory_iterator(const Path& path)
    {
        NativeIterator<fs::directory_iterator>* it = new NativeIterator<fs::directory_iterator>();
        it->_M_base                                = this;
        Path dir                                   = _M_path / path;
        it->_M_it                                  = fs::directory_iterator(dir.str());
        it->update_path();
        return it;
    }

    DirectoryIteratorInterface* NativeFileSystem::create_recursive_directory_iterator(const Path& path)
    {
        NativeIterator<fs::recursive_directory_iterator>* it = new NativeIterator<fs::recursive_directory_iterator>();
        it->_M_base                                          = this;
        Path dir                                             = _M_path / path;
        it->_M_it                                            = fs::recursive_directory_iterator(dir.str());
        it->update_path();
        return it;
    }


    NativeFileSystem::NativeFileSystem(const Path& directory) : _M_path(directory)
    {
        trinex_always_check(fs::is_directory(directory.str()), "Path to native file system must be directory!");
    }

    const Path& NativeFileSystem::path() const
    {
        return _M_path;
    }

    bool NativeFileSystem::is_read_only() const
    {
        return (fs::status(_M_path.str()).permissions() & fs::perms::owner_write) != fs::perms::owner_write;
    }

    File* NativeFileSystem::open(const Path& path, Flags<FileOpenMode> mode)
    {
        Path full_path = _M_path / path;

        std::ios_base::openmode open_mode = static_cast<std::ios_base::openmode>(0);
        bool is_read_only                 = !mode.has_any(Flags(FlagsOperator::Or, FileOpenMode::Out, FileOpenMode::Append));


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
            error_log("Native FS", "%s", std::strerror(errno));
        }
        return nullptr;
    }

    bool NativeFileSystem::create_dir(const Path& path)
    {
        return fs::create_directories((_M_path / path).str());
    }

    bool NativeFileSystem::remove(const Path& path)
    {
        return fs::remove((_M_path / path).str());
    }

    bool NativeFileSystem::copy(const Path& src, const Path& dest)
    {
        std::error_code code;
        fs::copy_file((_M_path / src).str(), (_M_path / dest).str(), code);

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
        fs::rename((_M_path / src).str(), (_M_path / dest).str(), code);

        if (code)
        {
            vfs_error("%s", code.message().c_str());
            return false;
        }
        return true;
    }

    bool NativeFileSystem::is_file_exist(const Path& path) const
    {
        return fs::exists((_M_path / path).str());
    }

    bool NativeFileSystem::is_file(const Path& file) const
    {
        return fs::is_regular_file((_M_path / file).str());
    }

    bool NativeFileSystem::is_dir(const Path& dir) const
    {
        return fs::is_directory((_M_path / dir).str());
    }

    NativeFileSystem::Type NativeFileSystem::type() const
    {
        return Type::Native;
    }

    Path NativeFileSystem::native_path(const Path& path) const
    {
        return _M_path / path;
    }
}// namespace Engine::VFS
