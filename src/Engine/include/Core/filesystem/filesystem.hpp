#pragma once
#include <Core/enums.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/flags.hpp>
#include <Core/implement.hpp>

namespace Engine::VFS
{
    class DirectoryIteratorInterface;
    class File;

    class ENGINE_EXPORT FileSystem
    {
    public:
        enum Type
        {
            Undefined = -1,
            Root      = 0,
            Virtual   = 1,
            Native    = 2,
        };

        using UnMountCallback = Function<void(FileSystem*)>;

    protected:
        Path _M_mount_point;
        UnMountCallback _M_on_unmount;
        virtual DirectoryIteratorInterface* create_directory_iterator(const Path& path)           = 0;
        virtual DirectoryIteratorInterface* create_recursive_directory_iterator(const Path& path) = 0;

    public:
        FORCE_INLINE FileSystem()          = default;
        FORCE_INLINE virtual ~FileSystem() = default;
        delete_copy_constructors(FileSystem);

        FORCE_INLINE const Path& mount_point() const
        {
            return _M_mount_point;
        }

        virtual const Path& path() const                               = 0;
        virtual bool is_read_only() const                              = 0;
        virtual File* open(const Path& path, Flags<FileOpenMode> mode) = 0;
        virtual bool create_dir(const Path& path)                      = 0;
        virtual bool remove(const Path& path)                          = 0;
        virtual bool copy(const Path& src, const Path& dest)           = 0;
        virtual bool rename(const Path& src, const Path& dest)         = 0;
        virtual bool is_file_exist(const Path& path) const             = 0;
        virtual bool is_file(const Path& file) const                   = 0;
        virtual bool is_dir(const Path& dir) const                     = 0;
        virtual Type type() const                                      = 0;
        virtual Path native_path(const Path& path) const               = 0;

        friend class RootFS;
    };
}// namespace Engine::VFS
