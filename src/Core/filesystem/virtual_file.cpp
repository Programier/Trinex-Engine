#include "vfs_log.hpp"
#include <Core/exception.hpp>
#include <Core/filesystem/virtual_file.hpp>
#include <Core/logger.hpp>
#include <zip.h>

namespace Engine::VFS
{

#define conv_zip_file_handle() reinterpret_cast<zip_file*>(m_zip_file_handle)
    VirtualFile::VirtualFile(void* handle, const Path& path) : m_zip_file_handle(handle), m_path(path)
    {
        trinex_always_check(zip_file_is_seekable(conv_zip_file_handle()), "File must be seekable!");
    }

    void VirtualFile::close()
    {
        if (m_zip_file_handle)
        {
            zip_fclose(conv_zip_file_handle());
            m_zip_file_handle = nullptr;
        }
    }

    bool VirtualFile::is_open() const
    {
        return m_zip_file_handle != nullptr;
    }

    bool VirtualFile::is_read_only() const
    {
        return true;
    }

    File::FilePosition VirtualFile::read_position(FileOffset offset, FileSeekDir dir)
    {
        int flags = dir == FileSeekDir::Begin ? SEEK_SET : dir == FileSeekDir::End ? SEEK_END : SEEK_CUR;
        zip_fseek(conv_zip_file_handle(), static_cast<zip_int64_t>(offset), flags);
        return read_position();
    }

    File::FilePosition VirtualFile::read_position()
    {
        return static_cast<File::FilePosition>(zip_ftell(conv_zip_file_handle()));
    }

    size_t VirtualFile::read(byte* buffer, size_t size)
    {
        return static_cast<size_t>(zip_fread(conv_zip_file_handle(), reinterpret_cast<void*>(buffer), size));
    }

    File::FilePosition VirtualFile::write_position(FileOffset offset, FileSeekDir dir)
    {
        return write_position();
    }

    File::FilePosition VirtualFile::write_position()
    {
        return 0;
    }

    size_t VirtualFile::write(const byte* buffer, size_t size)
    {
        vfs_error("Cannot write to virtual file!");
        return write_position();
    }

    VirtualFile::~VirtualFile()
    {
        close();
    }

    const Path& VirtualFile::path() const
    {
        return m_path;
    }
}// namespace Engine::VFS
