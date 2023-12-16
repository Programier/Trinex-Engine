#pragma once

#include <Core/buffer_manager.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/exception.hpp>
#include <fstream>
#include <memory>

namespace Engine
{
    using FileSeekDir = BufferSeekDir;

    class ENGINE_EXPORT FileWriter : public BufferWriter
    {
    public:
        using Pointer = FileWriter*;

    private:
        std::ofstream _M_file;
        String _M_filename;

    public:
        FileWriter();
        FileWriter(const String& filename, bool clear = true);
        FORCE_INLINE FileWriter(const Path& path, bool clear = true) : FileWriter(path.string(), clear)
        {}
        FileWriter(FileWriter&&);
        FileWriter& operator=(FileWriter&&);

        const String& filename() const;
        bool open(const String& filename, bool clear = true);
        FileWriter& close();

        bool write(const byte* data, size_t size) override;
        WritePos position() override;
        FileWriter& position(WritePos pos) override;
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
        std::ifstream _M_file;
        String _M_filename;

    public:
        FileReader();
        FileReader(const String& filename);
        FORCE_INLINE FileReader(const Path& path) : FileReader(path.string())
        {}
        FileReader(FileReader&&);
        FileReader& operator=(FileReader&&);

        const String& filename() const;
        bool open(const String& filename);
        FileReader& close();

        bool read(byte* data, size_t size) override;
        ReadPos position() override;
        FileReader& position(ReadPos pos) override;
        FileReader& offset(PosOffset offset, BufferSeekDir dir = BufferSeekDir::Current) override;
        bool is_open() const override;

        ~FileReader();
    };


    class ENGINE_EXPORT FileManager
    {

    public:
        using Files       = Set<String>;
        using Directories = Set<String>;

    private:
        Path _M_work_dir = "./";

    public:
        FileManager(const Path& directory = "./");
        FileManager(const FileManager& manager);
        FileManager(FileManager&&);
        static const FileManager* root_file_manager();

        FileManager& operator=(const FileManager&);
        FileManager& operator=(FileManager&&);

        static Path dirname_of(const Path& filename);
        static Path basename_of(const Path& filename);

        bool work_dir(const Path& directory);

        const Path& work_dir() const;

        FileReader::Pointer create_file_reader(const Path& filename) const;
        FileWriter::Pointer create_file_writer(const Path& filename, bool clear = true) const;

        bool remove(const Path& path, bool recursive = false) const;
        bool create_dir(const Path& path) const;
    };
}// namespace Engine
