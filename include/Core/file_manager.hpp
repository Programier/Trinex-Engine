#pragma once

#include <Core/buffer_manager.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/exception.hpp>
#include <fstream>
#include <memory>

namespace Engine
{
    namespace VFS
    {
        class File;
    }

    class ENGINE_EXPORT FileWriter : public BufferWriter
    {
    public:
        using Pointer = FileWriter*;

    private:
        VFS::File* m_file = nullptr;

    public:
        FileWriter();
        FileWriter(const Path& filename, bool clear = true);
        FileWriter(FileWriter&&);
        FileWriter& operator=(FileWriter&&);

        const Path& filename() const;
        bool open(const Path& filename, bool clear = true);
        FileWriter& close();

        bool write(const byte* data, size_t size) override;
        WritePos position() override;
        FileWriter& offset(PosOffset offset, BufferSeekDir dir = BufferSeekDir::Current) override;
        bool is_open() const override;
        FileWriter& clear() override;


        ~FileWriter();
    };


    class ENGINE_EXPORT FileReader : public BufferReader
    {
    public:
        using Pointer = FileReader*;

    private:
        VFS::File* m_file = nullptr;

    public:
        FileReader();
        FileReader(const Path& path);
        FileReader(FileReader&&);
        FileReader& operator=(FileReader&&);

        const Path& filename() const;
        bool open(const Path& path);
        FileReader& close();

        bool read(byte* data, size_t size) override;
        ReadPos position() override;
        FileReader& offset(PosOffset offset, BufferSeekDir dir = BufferSeekDir::Current) override;
        bool is_open() const override;

        String read_string(size_t len = -1);
        Buffer read_buffer(size_t len = -1);

        ~FileReader();
    };
}// namespace Engine
