#pragma once
#include <Core/filesystem/filesystem.hpp>
#include <Core/filesystem/path.hpp>

namespace Engine::VFS
{
    class ENGINE_EXPORT NativeFileSystem : public FileSystem
    {
    private:
        Path m_path;

    protected:
        DirectoryIteratorInterface* create_directory_iterator(const Path& path) override;
        DirectoryIteratorInterface* create_recursive_directory_iterator(const Path& path) override;


    public:
        delete_copy_constructors(NativeFileSystem);

        NativeFileSystem(const Path& directory);

        const Path& path() const override;
        Path native_path(const Path& path) const override;

        bool is_read_only() const override;
        File* open(const Path& path, Flags<FileOpenMode> mode) override;
        bool create_dir(const Path& path) override;
        bool remove(const Path& path) override;
        bool copy(const Path& src, const Path& dest) override;
        bool rename(const Path& src, const Path& dest) override;
        bool is_file_exist(const Path& path) const override;
        bool is_file(const Path& file) const override;
        bool is_dir(const Path& dir) const override;
        Type type() const override;
    };
}// namespace Engine::VFS
