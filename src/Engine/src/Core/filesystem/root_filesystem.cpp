#include "vfs_log.hpp"
#include <Core/filesystem/native_file_system.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Core/platform.hpp>
#include <filesystem>

namespace Engine
{
    ENGINE_EXPORT VFS::RootFS* rootfs()
    {
        return VFS::RootFS::instance();
    }

}// namespace Engine

namespace Engine::VFS
{
    bool RootFS::FileSystemCompare::operator()(const String& first, const String& second) const
    {
        return first.length() > second.length();
    }

    RootFS* RootFS::m_instance = nullptr;


    static void destroy_fs(FileSystem* fs)
    {
        delete fs;
    }

    RootFS::RootFS(const Path& native_path)
    {
        if (!native_path.empty())
        {
            mount("", new NativeFileSystem(native_path), destroy_fs);
            std::filesystem::current_path(native_path.str());
        }
        else
        {
            mount("", new NativeFileSystem("./"), destroy_fs);
        }

        // Mount platform hard drives
        Vector<Pair<Path, Path>> hard_drives = Platform::hard_drives();

        for (auto& pair : hard_drives)
        {
            mount(pair.first, new NativeFileSystem(pair.second));
        }
    }

    RootFS::~RootFS()
    {
        while (!m_file_systems.empty())
        {
            remove_fs(m_file_systems.begin());
        }
    }

    DirectoryIteratorInterface* RootFS::create_directory_iterator(const Path& path)
    {
        auto entry = find_filesystem(path);
        if (entry.first)
        {
            return entry.first->create_directory_iterator(entry.second);
        }
        return nullptr;
    }

    DirectoryIteratorInterface* RootFS::create_recursive_directory_iterator(const Path& path)
    {
        auto entry = find_filesystem(path);
        if (entry.first)
        {
            return entry.first->create_recursive_directory_iterator(entry.second);
        }
        return nullptr;
    }

    FileSystem* RootFS::remove_fs(const FileSystemMap::iterator& it)
    {
        FileSystem* system = it->second;
        m_file_systems.erase(it);
        system->m_mount_point   = {};
        UnMountCallback callback = std::move(system->m_on_unmount);

        if (callback)
        {
            callback(system);
        }

        return system;
    }

    bool RootFS::mount(const Path& mount_point, FileSystem* system, const UnMountCallback& callback)
    {
        if (system == nullptr)
        {
            vfs_error("Failed to mount nullptr system!");
        }

        if (m_file_systems.contains(mount_point))
        {
            vfs_error("Failed to create mount point '%s'. Mount point already exist!", mount_point.c_str());
            return false;
        }

        m_file_systems[mount_point] = system;
        system->m_mount_point       = mount_point;
        system->m_on_unmount        = callback;

        vfs_log("Mounted '%s' to '%s'", system->path().c_str(), mount_point.c_str());
        return system;
    }

    FileSystem* RootFS::unmount(const Path& mount_point)
    {
        auto it = m_file_systems.find(mount_point);

        if (it == m_file_systems.end())
        {
            return nullptr;
        }
        return remove_fs(it);
    }

    Pair<FileSystem*, Path> RootFS::find_filesystem(const Path& path) const
    {
        static auto next_symbol_of = [](const Path& path, const String& fs_path) -> char {
            if(fs_path.back() == Path::separator)
                return Path::separator;

            if (path.length() <= fs_path.length())
                return Path::separator;
            return path.str()[fs_path.length()];
        };

        for (auto& [fs_path, fs] : m_file_systems)
        {
            if (path.path().starts_with(fs_path) && (next_symbol_of(path, fs_path) == Path::separator || fs_path.empty()))
            {
                return {fs, path.relative(fs_path)};
            }
        }

        return {};
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

    File* RootFS::open(const Path& path, Flags<FileOpenMode> mode)
    {
        auto entry = find_filesystem(path);
        if (entry.first)
        {
            return entry.first->open(entry.second, mode);
        }
        return nullptr;
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
        return Type::Root;
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

    FileSystem::Type RootFS::filesystem_type_of(const Path& path) const
    {
        FileSystem* fs = filesystem_of(path);
        if (fs == nullptr)
            return Type::Undefined;
        return fs->type();
    }
}// namespace Engine::VFS
