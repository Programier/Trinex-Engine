#include <Core/engine_loading_controllers.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/predef.hpp>
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

        std::ios_base::openmode mode = std::ios_base::in;

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


    FileManager::FileManager(const String& directory)
    {
        work_dir(directory);
    }

    FileManager::FileManager(const FileManager& manager) = default;
    FileManager::FileManager(FileManager&&)              = default;

    String FileManager::dirname_of(const String& filename)
    {
        auto index = filename.find_last_of("/\\");
        if (index == String::npos)
        {
            return STR("./");
        }

        return filename.substr(0, index) + STR("/");
    }

    String FileManager::basename_of(const String& filename)
    {
        auto index = filename.find_last_of("/\\") + 1;
        return filename.substr(index, filename.length() - index);
    }

    String FileManager::make_dirname(String name)
    {
        if (name.back() != '/')
            name.push_back('/');
        return name;
    }

    FileManager& FileManager::next_dir(String directory)
    {
        return work_dir(work_dir() + make_dirname(directory));
    }

    FileManager& FileManager::private_update(void* data)
    {
        DIR* dir = reinterpret_cast<DIR*>(data);
        struct dirent* ent;

        _M_files.clear();
        _M_directories.clear();

        while ((ent = readdir(dir)) != NULL)
        {
            if (ent->d_type == DT_REG)
            {
                _M_files.insert(ent->d_name);
            }
            else if (ent->d_type == DT_DIR && std::strcmp(ent->d_name, ".") != 0 && std::strcmp(ent->d_name, "..") != 0)
            {
                _M_directories.insert(ent->d_name);
            }
        }

        return *this;
    }

    FileManager& FileManager::work_dir(String directory)
    {
        directory = make_dirname(directory);

        DIR* dir = nullptr;

        if ((dir = opendir(directory.c_str())) != NULL)
        {
            _M_work_dir = directory;
            private_update(dir);
            closedir(dir);
        }
        else
        {
            logger->error("FileManager: Directory '%s' not found!", directory.c_str());
        }
        return *this;
    }

    FileManager& FileManager::update()
    {
        DIR* dir = nullptr;
        if ((dir = opendir(_M_work_dir.c_str())) != NULL)
        {
            private_update(dir);
            closedir(dir);
        }
        else
        {
            throw EngineException("FileManager: Directory not found!");
        }
        return *this;
    }

    const String& FileManager::work_dir() const
    {
        return _M_work_dir;
    }

    const FileManager* FileManager::root_file_manager()
    {
        return root_manager();
    }

    FileManager& FileManager::operator=(const FileManager&) = default;
    FileManager& FileManager::operator=(FileManager&&)      = default;

    const FileManager::Files& FileManager::files() const
    {
        return _M_files;
    }

    const FileManager::Directories& FileManager::directories() const
    {
        return _M_directories;
    }

    FileReader::Pointer FileManager::create_file_reader(const String& filename) const
    {
        FileReader* file_reader = new FileReader(_M_work_dir + filename);
        if (!file_reader->is_open())
        {
            delete file_reader;
            file_reader = nullptr;
        }

        return FileReader::Pointer(file_reader);
    }

    FileWriter::Pointer FileManager::create_file_writer(const String& filename, bool clear) const
    {
        FileWriter* file_writer = new FileWriter(_M_work_dir + filename, clear);
        if (!file_writer->is_open())
        {
            delete file_writer;
            file_writer = nullptr;
        }
        return FileWriter::Pointer(file_writer);
    }

    TextFileReader::Pointer FileManager::create_text_file_reader(const String& filename) const
    {
        TextFileReader* file_reader = new TextFileReader(_M_work_dir + filename);
        if (!file_reader->is_open())
        {
            delete file_reader;
            file_reader = nullptr;
        }

        return TextFileReader::Pointer(file_reader);
    }

    TextFileWriter::Pointer FileManager::create_text_file_writer(const String& filename, bool clear) const
    {
        TextFileWriter* file_writer = new TextFileWriter(_M_work_dir + filename, clear);
        if (!file_writer->is_open())
        {
            delete file_writer;
            file_writer = nullptr;
        }
        return TextFileWriter::Pointer(file_writer);
    }


    const FileManager& FileManager::remove_dir(const String& name, bool recursive) const
    {
        String full_path = _M_work_dir + make_dirname(name);

#if PLATFORM_WINDOWS
        String command = Strings::format("rd {} /q {}", (recursive ? "-r" : ""), full_path);
#else
        String command = Strings::format("rm {} {}", (recursive ? "-r" : ""), full_path);
#endif
        std::system(command.c_str());
        return *this;
    }

    const FileManager& FileManager::remove_file(const String& name) const
    {
        String full_path = _M_work_dir + name;
#if PLATFORM_WINDOWS
        String command = Strings::format("del {}", full_path);
#else
        String command = Strings::format("rm {}", full_path);
#endif
        std::system(command.c_str());
        return *this;
    }

    const FileManager& FileManager::create_dir(const String& name) const
    {
        String full_path = _M_work_dir + make_dirname(name);
#if PLATFORM_WINDOWS
        String command = Strings::format("mkdir {}", full_path);
#else
        String command = Strings::format("mkdir -p {}", full_path);
#endif
        std::system(command.c_str());
        return *this;
    }
}// namespace Engine
