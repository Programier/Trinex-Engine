#include <Core/filesystem/directory_iterator.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/filesystem/root_filesystem.hpp>


namespace Engine::VFS
{
    DirectoryIterator::DirectoryIterator() : _M_interface(nullptr)
    {}

    DirectoryIterator::DirectoryIterator(const Path& path) : _M_interface(rootfs()->create_directory_iterator(path))
    {}

    DirectoryIterator::DirectoryIterator(const DirectoryIterator& other)
        : _M_interface(other._M_interface ? other._M_interface->copy() : nullptr)
    {}

    DirectoryIterator::DirectoryIterator(DirectoryIterator&& other) : _M_interface(other._M_interface)
    {
        other._M_interface = nullptr;
    }

    DirectoryIterator& DirectoryIterator::operator=(const DirectoryIterator& other)
    {
        if (this == &other)
            return *this;

        if (_M_interface)
            delete _M_interface;

        _M_interface = other._M_interface ? other._M_interface->copy() : nullptr;
        return *this;
    }

    DirectoryIterator& DirectoryIterator::operator=(DirectoryIterator&& other)
    {
        if (this == &other)
            return *this;

        if (_M_interface)
            delete _M_interface;

        _M_interface       = other._M_interface;
        other._M_interface = nullptr;
        return *this;
    }

    DirectoryIterator& DirectoryIterator::begin()
    {
        return *this;
    }

    DirectoryIterator& DirectoryIterator::end()
    {
        static DirectoryIterator _M_end;
        return _M_end;
    }

    DirectoryIterator& DirectoryIterator::operator++()
    {
        if (_M_interface)
        {
            _M_interface->next();
        }
        return *this;
    }

    const Path& DirectoryIterator::operator*()
    {
        static const Path default_path;
        return _M_interface && _M_interface->is_valid() ? _M_interface->path() : default_path;
    }

    bool DirectoryIterator::operator!=(const DirectoryIterator& other) const
    {
        return !((*this) == other);
    }

    bool DirectoryIterator::operator==(const DirectoryIterator& other) const
    {
        if (_M_interface == other._M_interface)
            return true;

        if (_M_interface == nullptr)
            return !other._M_interface->is_valid();

        if (other._M_interface == nullptr)
            return !_M_interface->is_valid();

        if (_M_interface->type() != other._M_interface->type())
            return false;

        return _M_interface->is_equal(other._M_interface);
    }


    RecursiveDirectoryIterator::RecursiveDirectoryIterator() : _M_interface(nullptr)
    {}

    RecursiveDirectoryIterator::RecursiveDirectoryIterator(const Path& path)
        : _M_interface(rootfs()->create_recursive_directory_iterator(path))
    {}

    RecursiveDirectoryIterator::RecursiveDirectoryIterator(const RecursiveDirectoryIterator& other)
        : _M_interface(other._M_interface ? other._M_interface->copy() : nullptr)
    {}

    RecursiveDirectoryIterator::RecursiveDirectoryIterator(RecursiveDirectoryIterator&& other) : _M_interface(other._M_interface)
    {
        other._M_interface = nullptr;
    }

    RecursiveDirectoryIterator& RecursiveDirectoryIterator::operator=(const RecursiveDirectoryIterator& other)
    {
        if (this == &other)
            return *this;

        if (_M_interface)
            delete _M_interface;

        _M_interface = other._M_interface ? other._M_interface->copy() : nullptr;
        return *this;
    }

    RecursiveDirectoryIterator& RecursiveDirectoryIterator::operator=(RecursiveDirectoryIterator&& other)
    {
        if (this == &other)
            return *this;

        if (_M_interface)
            delete _M_interface;

        _M_interface       = other._M_interface;
        other._M_interface = nullptr;
        return *this;
    }

    RecursiveDirectoryIterator& RecursiveDirectoryIterator::begin()
    {
        return *this;
    }

    RecursiveDirectoryIterator& RecursiveDirectoryIterator::end()
    {
        static RecursiveDirectoryIterator _M_end;
        return _M_end;
    }

    RecursiveDirectoryIterator& RecursiveDirectoryIterator::operator++()
    {
        if (_M_interface)
        {
            _M_interface->next();
        }
        return *this;
    }

    const Path& RecursiveDirectoryIterator::operator*()
    {
        static const Path default_path;
        return _M_interface && _M_interface->is_valid() ? _M_interface->path() : default_path;
    }

    bool RecursiveDirectoryIterator::operator!=(const RecursiveDirectoryIterator& other) const
    {
        return !((*this) == other);
    }

    bool RecursiveDirectoryIterator::operator==(const RecursiveDirectoryIterator& other) const
    {
        if (_M_interface == other._M_interface)
            return true;

        if (_M_interface == nullptr)
            return !other._M_interface->is_valid();

        if (other._M_interface == nullptr)
            return !_M_interface->is_valid();

        if (_M_interface->type() != other._M_interface->type())
            return false;

        return _M_interface->is_equal(other._M_interface);
    }
}// namespace Engine::VFS
