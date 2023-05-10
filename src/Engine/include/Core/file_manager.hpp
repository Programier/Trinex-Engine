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
        String _M_work_dir = "./";
        Files _M_files;
        Directories _M_directories;

        FileManager& private_update(void* data);

    public:
        FileManager(const String& directory = "./");
        FileManager(const FileManager& manager);
        FileManager(FileManager&&);
        static const FileManager* root_file_manager();

        FileManager& operator=(const FileManager&);
        FileManager& operator=(FileManager&&);

        static String dirname_of(const String& filename);
        static String basename_of(const String& filename);
        static String make_dirname(String name);


        FileManager& next_dir(String directory);
        FileManager& work_dir(String directory);
        FileManager& update();
        const String& work_dir() const;

        const Files& files() const;
        const Directories& directories() const;
        FileReader::Pointer create_file_reader(const String& filename) const;
        FileWriter::Pointer create_file_writer(const String& filename, bool clear = true) const;
        TextFileReader::Pointer create_text_file_reader(const String& filename) const;
        TextFileWriter::Pointer create_text_file_writer(const String& filename, bool clear = true) const;
        const FileManager& remove_dir(const String& name, bool recursive = false) const;
        const FileManager& remove_file(const String& name) const;
        const FileManager& create_dir(const String& name) const;
    };
}// namespace Engine
