#pragma once
#include <Core/filesystem/filesystem.hpp>

namespace Engine::VFS
{
    class ENGINE_EXPORT VirtualFileSystem : public FileSystem
    {
    private:
        void* _M_zip_handle = nullptr;
        Path _M_path;


    protected:
        DirectoryIteratorInterface* create_directory_iterator(const Path& path) override;
        DirectoryIteratorInterface* create_recursive_directory_iterator(const Path& path) override;

        char last_symbol_of_path(const Path& path) const;

    public:
        VirtualFileSystem(const Path& path);
        ~VirtualFileSystem();

        const Path& path() const override;
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
        Path native_path(const Path& path) const override;
    };
}// namespace Engine::VFS
