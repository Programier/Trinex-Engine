#pragma once

#include <Core/buffer_manager.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/exception.hpp>
#include <Core/export.hpp>
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
        FileWriter(FileWriter&&);
        FileWriter& operator=(FileWriter&&);

        bool open(const String& filename, bool clear = true);
        FileWriter& close();
        Stream& stream() override;
        bool is_open() const override;
        FileWriter& clear() override;
        const String& filename() const;

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
        FileReader(FileReader&&);
        FileReader& operator=(FileReader&&);

        Stream& stream() override;
        bool open(const String& filename);
        FileReader& close();
        bool is_open() const override;
        const String& filename() const;
        ~FileReader();
    };


    class ENGINE_EXPORT TextFileWriter : public TextBufferWriter
    {
    public:
        using Pointer = TextFileWriter*;

    private:
        std::ofstream _M_file;
        String _M_filename;

    public:
        TextFileWriter();
        TextFileWriter(const String& filename, bool clear = true);
        TextFileWriter(TextFileWriter&&);
        TextFileWriter& operator=(TextFileWriter&&);

        bool open(const String& filename, bool clear = true);
        TextFileWriter& close();
        Stream& stream() override;
        bool is_open() const override;
        TextFileWriter& clear() override;
        const String& filename() const;

        ~TextFileWriter();
    };


    class ENGINE_EXPORT TextFileReader : public TextBufferReader
    {
    public:
        using Pointer = TextFileReader*;

    private:
        std::ifstream _M_file;
        String _M_filename;

    public:
        TextFileReader();
        TextFileReader(const String& filename);
        TextFileReader(TextFileReader&&);
        TextFileReader& operator=(TextFileReader&&);

        Stream& stream() override;
        bool open(const String& filename);
        TextFileReader& close();
        bool is_open() const override;
        const String& filename() const;
        ~TextFileReader();
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
        TextFileReader::Pointer create_text_file_reader(const Path& filename) const;
        TextFileWriter::Pointer create_text_file_writer(const Path& filename, bool clear = true) const;

        bool remove(const Path& path, bool recursive = false) const;
        bool create_dir(const Path& path) const;
    };
}// namespace Engine
