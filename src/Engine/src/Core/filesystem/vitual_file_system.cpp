#include "vfs_log.hpp"
#include <Core/constants.hpp>
#include <Core/exception.hpp>
#include <Core/filesystem/directory_iterator.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/filesystem/virtual_file.hpp>
#include <Core/filesystem/virtual_file_system.hpp>
#include <cstring>
#include <filesystem>
#include <zip.h>

namespace fs = std::filesystem;

namespace Engine::VFS
{
#define conv_zip_handle() reinterpret_cast<zip*>(m_zip_handle)

    static void collect_entries(zip* archive, Vector<Path>& out, const Path& start_path, bool recurse, const Path& mount)
    {
        for (zip_int64_t i = 0, j = zip_get_num_entries(archive, 0); i < j; ++i)
        {
            zip_stat_t stat;
            if (zip_stat_index(archive, i, 0, &stat) == 0)
            {
                Path entry_path(stat.name);

                if (entry_path == start_path)
                    continue;

                if (recurse)
                {
                    if (entry_path.starts_with(start_path))
                        out.emplace_back(mount / entry_path);
                }
                else
                {
                    if (entry_path.starts_with(start_path) &&
                        entry_path.str().find(Path::separator, start_path.length() + 1) == String::npos)
                        out.emplace_back(mount / entry_path);
                }
            }
        }
    }

    struct VirtualDirectoryIterator : public DirectoryIteratorInterface {
        Vector<Path> m_paths;
        Index m_index = 0;


        void next() override
        {
            ++m_index;
        }

        const Path& path() override
        {
            return m_paths[m_index];
        }

        bool is_valid() const override
        {
            return m_index < m_paths.size();
        }

        DirectoryIteratorInterface* copy() override
        {
            return new VirtualDirectoryIterator(*this);
        }

        Type type() const override
        {
            return Type::Virtual;
        }

        bool is_equal(DirectoryIteratorInterface* interface) override
        {
            VirtualDirectoryIterator* second = reinterpret_cast<VirtualDirectoryIterator*>(interface);
            return m_paths[m_index] == second->m_paths[second->m_index];
        }
    };


    VirtualFileSystem::VirtualFileSystem(const Path& path) : m_path(path)
    {
        trinex_always_check(path.extension() != Constants::translation_config_extension || !fs::is_regular_file(path.str()),
                            "Path to native file system must be directory!");
        m_zip_handle = zip_open(path.c_str(), ZIP_RDONLY, nullptr);
    }

    VirtualFileSystem::~VirtualFileSystem()
    {
        if (m_zip_handle)
        {
            zip_close(conv_zip_handle());
            m_zip_handle = nullptr;
        }
    }


    static DirectoryIteratorInterface* create_iterator_interface(void* m_zip_handle, const Path& path, bool recursive, const Path& mount)
    {
        Vector<Path> entries;
        collect_entries(conv_zip_handle(), entries, path, false, mount);

        if (entries.empty())
            return nullptr;

        VirtualDirectoryIterator* iterator = new VirtualDirectoryIterator();
        iterator->m_index                 = 0;
        iterator->m_paths                 = std::move(entries);
        return iterator;
    }

    DirectoryIteratorInterface* VirtualFileSystem::create_directory_iterator(const Path& path)
    {
        return create_iterator_interface(m_zip_handle, path, false, mount_point());
    }

    DirectoryIteratorInterface* VirtualFileSystem::create_recursive_directory_iterator(const Path& path)
    {
        return create_iterator_interface(m_zip_handle, path, true, mount_point());
    }

    char VirtualFileSystem::last_symbol_of_path(const Path& path) const
    {
        zip_stat_t stat;
        if (zip_stat(conv_zip_handle(), path.c_str(), 0, &stat) == -1)
            return 0;

        auto len = std::strlen(stat.name);
        if (len == 0)
            return 0;

        return stat.name[len - 1];
    }

    const Path& VirtualFileSystem::path() const
    {
        return m_path;
    }

    bool VirtualFileSystem::is_read_only() const
    {
        return true;
    }

    File* VirtualFileSystem::open(const Path& path, Flags<FileOpenMode> mode)
    {
        zip_file* file = zip_fopen(conv_zip_handle(), path.c_str(), 0);
        if (file == nullptr)
            return nullptr;

        return new VirtualFile(file, path);
    }

    bool VirtualFileSystem::create_dir(const Path& path)
    {
        vfs_error("Virtual file system is read only!");
        return false;
    }

    bool VirtualFileSystem::remove(const Path& path)
    {
        vfs_error("Virtual file system is read only!");
        return false;
    }

    bool VirtualFileSystem::copy(const Path& src, const Path& dest)
    {
        vfs_error("Virtual file system is read only!");
        return false;
    }

    bool VirtualFileSystem::rename(const Path& src, const Path& dest)
    {
        vfs_error("Virtual file system is read only!");
        return false;
    }

    bool VirtualFileSystem::is_file_exist(const Path& path) const
    {
        return last_symbol_of_path(path) != 0;
    }

    bool VirtualFileSystem::is_file(const Path& file) const
    {
        char last = last_symbol_of_path(file);
        return last != 0 && last != '/';
    }

    bool VirtualFileSystem::is_dir(const Path& dir) const
    {
        return last_symbol_of_path(dir) == '/';
    }

    FileSystem::Type VirtualFileSystem::type() const
    {
        return Type::Virtual;
    }

    Path VirtualFileSystem::native_path(const Path& path) const
    {
        // No native path to virtual file!
        return {};
    }

    static void add_folder_to_zip(zip* archive, const fs::path& folder_path, const fs::path& base_folder)
    {
        for (const auto& entry : fs::directory_iterator(folder_path))
        {
            const auto& file_path = entry.path();
            const auto& zip_path  = base_folder / file_path.filename();

            if (fs::is_directory(file_path))
            {
                zip_dir_add(archive, zip_path.string().c_str(), 0);
                add_folder_to_zip(archive, file_path, zip_path);
            }
            else
            {
                zip_source_t* source = zip_source_file(archive, file_path.string().c_str(), 0, 0);
                if (source)
                {
                    zip_int64_t file_index = zip_file_add(archive, zip_path.string().c_str(), source, ZIP_FL_OVERWRITE);
                    zip_set_file_compression(archive, file_index, ZIP_CM_STORE, 0);
                }
                else
                {
                    vfs_error("Failed to add file '%s' to fs", file_path.string().c_str());
                }
            }
        }
    }

    bool RootFS::pack_native_folder(const Path& native, const Path& _virtual_fs, const StringView& password) const
    {
        Path virtual_fs = _virtual_fs + Constants::virtual_file_system_extension;

        // Check paths
        if (filesystem_type_of(native) != Type::Native)
        {
            vfs_error("Cannot pack resources from non-native sources");
            return false;
        }

        if (filesystem_type_of(virtual_fs) != Type::Native)
        {
            vfs_error("Virtual FS must be located on native fs");
            return false;
        }

        if (!is_dir(native))
        {
            vfs_error("Cannot pack non-dir sources!");
            return false;
        }

        zip* archive = zip_open(native_path(virtual_fs).c_str(), ZIP_CREATE | ZIP_TRUNCATE, NULL);

        if (!archive)
        {
            vfs_error("Failed to create virtual file system!");
            return false;
        }

        {
            auto path = fs::path(native_path(native).str());
            add_folder_to_zip(archive, path, "");
            zip_close(archive);
        }

        return true;
    }
}// namespace Engine::VFS
