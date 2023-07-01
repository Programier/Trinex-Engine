#include <Core/engine_loading_controllers.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <cstring>
#include <dirent/dirent.h>
#include <sys/stat.h>


namespace Engine
{
    FileWriter::FileWriter() = default;
    FileWriter::FileWriter(const String& filename, bool clear)
    {
        open(filename, clear);
    }

    FileWriter::FileWriter(FileWriter&&)            = default;
    FileWriter& FileWriter::operator=(FileWriter&&) = default;

    bool FileWriter::open(const String& filename, bool clear)
    {
        std::ios_base::openmode mode =
                std::ios_base::binary | std::ios_base::out | (clear ? std::ios_base::trunc : std::ios_base::ate);

        close();
        _M_file.open(filename, mode);
        bool is_opened = _M_file.is_open();
        _M_filename    = is_opened ? filename : "";
        return is_opened;
    }

    FileWriter& FileWriter::close()
    {
        if (_M_file.is_open())
        {
            _M_file.close();
        }
        return *this;
    }


    BufferWriter::Stream& FileWriter::stream()
    {
        return _M_file;
    }

    bool FileWriter::is_open() const
    {
        return _M_file.is_open();
    }


    FileWriter& FileWriter::clear()
    {
        open(_M_filename, true);
        return *this;
    }

    const String& FileWriter::filename() const
    {
        return _M_filename;
    }

    FileWriter::~FileWriter()
    {
        close();
    }


    FileReader::FileReader() = default;
    FileReader::FileReader(const String& filename)
    {
        open(filename);
    }

    FileReader::FileReader(FileReader&&)            = default;
    FileReader& FileReader::operator=(FileReader&&) = default;


    BufferReader::Stream& FileReader::stream()
    {
        return _M_file;
    }

    bool FileReader::open(const String& filename)
    {
        close();

        std::ios_base::openmode mode = std::ios_base::in | std::ios_base::binary;

        _M_file.open(filename, mode);
        (void) size();
        bool is_opened = is_open();

        _M_filename = is_opened ? filename : "";
        return is_opened;
    }

    FileReader& FileReader::close()
    {
        if (_M_file.is_open())
        {
            _M_file.close();
        }
        return *this;
    }

    bool FileReader::is_open() const
    {
        return _M_file.is_open();
    }

    const String& FileReader::filename() const
    {
        return _M_filename;
    }

    FileReader::~FileReader()
    {
        close();
    }

    TextFileWriter::TextFileWriter() = default;
    TextFileWriter::TextFileWriter(const String& filename, bool clear)
    {
        open(filename, clear);
    }

    TextFileWriter::TextFileWriter(TextFileWriter&&)            = default;
    TextFileWriter& TextFileWriter::operator=(TextFileWriter&&) = default;

    bool TextFileWriter::open(const String& filename, bool clear)
    {
        std::ios_base::openmode mode = std::ios_base::out | (clear ? std::ios_base::trunc : std::ios_base::ate);

        close();
        _M_file.open(filename, mode);
        bool is_opened = _M_file.is_open();
        _M_filename    = is_opened ? filename : "";
        return is_opened;
    }

    TextFileWriter& TextFileWriter::close()
    {
        if (_M_file.is_open())
        {
            _M_file.close();
        }
        return *this;
    }


    BufferWriter::Stream& TextFileWriter::stream()
    {
        return _M_file;
    }

    bool TextFileWriter::is_open() const
    {
        return _M_file.is_open();
    }


    TextFileWriter& TextFileWriter::clear()
    {
        open(_M_filename, true);
        return *this;
    }

    const String& TextFileWriter::filename() const
    {
        return _M_filename;
    }

    TextFileWriter::~TextFileWriter()
    {
        close();
    }


    TextFileReader::TextFileReader() = default;
    TextFileReader::TextFileReader(const String& filename)
    {
        open(filename);
    }

    TextFileReader::TextFileReader(TextFileReader&&)            = default;
    TextFileReader& TextFileReader::operator=(TextFileReader&&) = default;


    BufferReader::Stream& TextFileReader::stream()
    {
        return _M_file;
    }

    bool TextFileReader::open(const String& filename)
    {
        close();

        std::ios_base::openmode mode = std::ios_base::binary | std::ios_base::in;

        _M_file.open(filename, mode);
        (void) size();
        bool is_opened = is_open();

        _M_filename = is_opened ? filename : "";
        return is_opened;
    }

    TextFileReader& TextFileReader::close()
    {
        if (_M_file.is_open())
        {
            _M_file.close();
        }
        return *this;
    }

    bool TextFileReader::is_open() const
    {
        return _M_file.is_open();
    }

    const String& TextFileReader::filename() const
    {
        return _M_filename;
    }

    TextFileReader::~TextFileReader()
    {
        close();
    }


    static FileManager* root_manager(bool create_if_not_exists = true)
    {
        static FileManager* manager = nullptr;
        if (!manager && create_if_not_exists)
            manager = new FileManager("./");
        return manager;
    }

    static void on_destroy()
    {
        FileManager* root_file_manager = root_manager(false);
        if (root_file_manager)
            delete root_file_manager;
    }

    DestroyController controller(on_destroy);


    FileManager::FileManager(const Path& directory)
    {
        work_dir(directory);
    }

    FileManager::FileManager(const FileManager& manager) = default;
    FileManager::FileManager(FileManager&&)              = default;

    Path FileManager::dirname_of(const Path& filename)
    {
        return filename.parent_path();
    }

    Path FileManager::basename_of(const Path& filename)
    {
        return filename.filename();
    }


    bool FileManager::work_dir(const Path& directory)
    {
        Path new_path = _M_work_dir / directory;
        if (FS::is_directory(new_path))
        {
            _M_work_dir = std::move(new_path);
            return true;
        }
        else
        {
            error_log("FileManager: Directory '%s' not found!", directory.c_str());
            return false;
        }
    }


    const Path& FileManager::work_dir() const
    {
        return _M_work_dir;
    }

    const FileManager* FileManager::root_file_manager()
    {
        return root_manager();
    }

    FileManager& FileManager::operator=(const FileManager&) = default;
    FileManager& FileManager::operator=(FileManager&&)      = default;

    FileReader::Pointer FileManager::create_file_reader(const Path& filename) const
    {
        FileReader* file_reader = new FileReader(_M_work_dir / filename);
        if (!file_reader->is_open())
        {
            delete file_reader;
            file_reader = nullptr;
        }

        return FileReader::Pointer(file_reader);
    }

    FileWriter::Pointer FileManager::create_file_writer(const Path& filename, bool clear) const
    {
        FileWriter* file_writer = new FileWriter(_M_work_dir / filename, clear);
        if (!file_writer->is_open())
        {
            delete file_writer;
            file_writer = nullptr;
        }
        return FileWriter::Pointer(file_writer);
    }

    TextFileReader::Pointer FileManager::create_text_file_reader(const Path& filename) const
    {
        TextFileReader* file_reader = new TextFileReader((_M_work_dir / filename).string());
        if (!file_reader->is_open())
        {
            delete file_reader;
            file_reader = nullptr;
        }

        return TextFileReader::Pointer(file_reader);
    }

    TextFileWriter::Pointer FileManager::create_text_file_writer(const Path& filename, bool clear) const
    {
        TextFileWriter* file_writer = new TextFileWriter((_M_work_dir / filename).string(), clear);
        if (!file_writer->is_open())
        {
            delete file_writer;
            file_writer = nullptr;
        }
        return TextFileWriter::Pointer(file_writer);
    }


    bool FileManager::remove(const Path& path, bool recursive) const
    {
        if (FS::exists(path))
        {
            try
            {
                if (FS::is_directory(path))
                {
                    if (recursive)
                    {
                        FS::remove_all(path);
                    }
                    else
                    {
                        FS::remove(path);
                    }
                }
                else
                {
                    FS::remove(path);
                }
            }
            catch (const std::exception& e)
            {
                error_log("FileSystem: Failed to remove '%s': %s", path.c_str(), e.what());
                return false;
            }
        }
        else
        {
            error_log("FileSystem: Error: '%s' does not exist", path.c_str());
            return false;
        }

        return true;
    }

    bool FileManager::create_dir(const Path& name) const
    {
        Path path = _M_work_dir / name;

        try
        {
            if (FS::exists(path) && FS::is_directory(path))
                return true;

            if (!FS::create_directory(path))
            {
                error_log("FileManager: Failed to create directory: '%s'", path.c_str());
                return false;
            }
        }
        catch (const std::exception& e)
        {
            error_log("FileSystem: Failed to create directory '%s': %s", e.what());
            return false;
        }
        return true;
    }
}// namespace Engine
