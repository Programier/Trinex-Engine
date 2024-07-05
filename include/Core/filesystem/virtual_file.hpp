#pragma once
#include <Core/filesystem/file.hpp>

namespace Engine::VFS
{
    class ENGINE_EXPORT VirtualFile : public File
    {
    private:
        void* m_zip_file_handle = nullptr;
        Path m_path;

        VirtualFile(void* handle, const Path& path);

    public:
        const Path& path() const override;
        bool is_read_only() const override;
        void close() override;
        bool is_open() const override;
        FilePosition read_position(FileOffset offset, FileSeekDir dir) override;
        FilePosition read_position() override;
        FilePosition write_position(FileOffset offset, FileSeekDir dir) override;
        FilePosition write_position() override;
        size_t read(byte* buffer, size_t size) override;
        size_t write(const byte* buffer, size_t size) override;

        ~VirtualFile();
        friend class VirtualFileSystem;
    };
}// namespace Engine::VFS
