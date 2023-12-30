#include <Core/engine_loading_controllers.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <cstring>
#include <dirent/dirent.h>
#include <sys/stat.h>


namespace Engine
{
    static FileManager* _M_root_manager = nullptr;

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


    bool FileWriter::write(const byte* data, size_t size)
    {
        return static_cast<bool>(_M_file.write(reinterpret_cast<const char*>(data), size));
    }


    FileWriter::WritePos FileWriter::position()
    {
        if (!is_open())
            return 0;
        return static_cast<WritePos>(_M_file.tellp());
    }

    FileWriter& FileWriter::offset(PosOffset offset, BufferSeekDir dir)
    {
        std::ios_base::seekdir _M_dir =
                (dir == BufferSeekDir::Begin ? std::ios_base::beg
                                             : (dir == BufferSeekDir::Current ? std::ios_base::cur : std::ios_base::end));
        _M_file.seekp(static_cast<std::ostream::pos_type>(offset), _M_dir);
        return *this;
    }


    FileWriter::~FileWriter()
    {
        close();
    }


    FileReader::FileReader() = default;
    FileReader::FileReader(const Path& filename)
    {
        open(filename);
    }

    FileReader::FileReader(FileReader&&)            = default;
    FileReader& FileReader::operator=(FileReader&&) = default;


    bool FileReader::open(const Path& path)
    {
        close();

        std::ios_base::openmode mode = std::ios_base::in | std::ios_base::binary;

        _M_file.open(path, mode);
        (void) size();
        bool is_opened = is_open();

        _M_path = is_opened ? path : "";
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

    String FileReader::to_string(size_t len)
    {
        len = glm::min(len, size());
        String result(len, 0);
        read(reinterpret_cast<byte*>(result.data()), len);
        return result;
    }

    Buffer FileReader::read_buffer(size_t len)
    {
        len = glm::min(len, size());
        Buffer result(len, 0);
        read(reinterpret_cast<byte*>(result.data()), len);
        return result;
    }

    const Path& FileReader::path() const
    {
        return _M_path;
    }


    bool FileReader::read(byte* data, size_t size)
    {
        return static_cast<bool>(_M_file.read(reinterpret_cast<char*>(data), size));
    }


    FileReader::ReadPos FileReader::position()
    {
        if (!is_open())
            return 0;
        return static_cast<ReadPos>(_M_file.tellg());
    }


    FileReader& FileReader::offset(PosOffset offset, BufferSeekDir dir)
    {
        std::ios_base::seekdir _M_dir =
                (dir == BufferSeekDir::Begin ? std::ios_base::beg
                                             : (dir == BufferSeekDir::Current ? std::ios_base::cur : std::ios_base::end));
        _M_file.seekg(static_cast<std::ostream::pos_type>(offset), _M_dir);

        return *this;
    }

    FileReader::~FileReader()
    {
        close();
    }


    static FileManager* root_manager(bool create_if_not_exists = true)
    {
        if (!_M_root_manager && create_if_not_exists)
            _M_root_manager = new FileManager("./");
        return _M_root_manager;
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
            if (this == _M_root_manager)
            {
                info_log("FileManager", "Setting root manager to '%s'", _M_work_dir.string().c_str());
                std::filesystem::current_path(_M_work_dir);
            }
            return true;
        }
        else
        {
            error_log("FileManager", "Directory '%s' not found!", directory.c_str());
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
                error_log("FileSystem", "Failed to remove '%s': %s", path.c_str(), e.what());
                return false;
            }
        }
        else
        {
            error_log("FileSystem", "Error: '%s' does not exist", path.c_str());
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
                error_log("FileManager", "Failed to create directory: '%s'", path.c_str());
                return false;
            }
        }
        catch (const std::exception& e)
        {
            error_log("FileSystem", "Failed to create directory '%s': %s", e.what());
            return false;
        }
        return true;
    }
}// namespace Engine
