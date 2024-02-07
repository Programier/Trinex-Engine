#pragma once
#include <Core/filesystem/file.hpp>
#include <Core/implement.hpp>
#include <fstream>

namespace Engine::VFS
{
    class ENGINE_EXPORT NativeFile final : public File
    {
    private:
        Path _M_path;
        Path _M_native_path;
        std::fstream _M_stream;
        bool _M_is_read_only;

        NativeFile(const Path& path, const Path& native_path, std::fstream&& stream, bool is_read_only);

    public:
        delete_copy_constructors(NativeFile);

        const Path& path() const override;
        const Path& native_path() const;
        bool is_read_only() const override;
        void close() override;
        bool is_open() const override;
        FilePosition write_position(FileOffset offset, FileSeekDir dir) override;
        FilePosition write_position() override;
        FilePosition read_position(FileOffset offset, FileSeekDir dir) override;
        FilePosition read_position() override;
        size_t read(byte* buffer, size_t size) override;
        size_t write(const byte* buffer, size_t size) override;

        friend class NativeFileSystem;
    };
}// namespace Engine::VFS
