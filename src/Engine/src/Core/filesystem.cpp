#include <Core/constants.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/filesystem.hpp>
#include <Core/logger.hpp>
#include <Core/platform.hpp>
#include <filesystem>


namespace Engine::VFS
{
#define vfs_info(...) info_log("VFS", __VA_ARGS__)
#define vfs_warning(...) warning_log("VFS", __VA_ARGS__)
#define vfs_error(...) error_log("VFS", __VA_ARGS__)
#define vfs_debug(...) debug_log("VFS", __VA_ARGS__)


    static const char* current_dir = ".";
    static const char* parent_dir  = "..";
#if PLATFORM_WINDOWS
    const char Path::separator = '\\';
#else
    const char Path::separator = '/';
#endif

    default_copy_constructors_cpp(Path);

    Path::Path() = default;
    Path::Path(const String& path) : _M_path(path)
    {
        simplify();
    }

    Path::Path(String&& path) : _M_path(std::move(path))
    {
        simplify();
    }

    Path::Path(const char* path) : Path(String(path))
    {}

    Path& Path::operator=(const String& path)
    {
        _M_path = path;
        return simplify();
    }

    Path& Path::operator=(String&& path)
    {
        _M_path = std::move(path);
        return simplify();
    }

    Path& Path::operator=(const char* path)
    {
        return (*this) = String(path);
    }

    Path& Path::operator+=(const char* line)
    {
        _M_path += line;
        return simplify();
    }

    Path& Path::operator+=(const String& line)
    {
        _M_path += line;
        return simplify();
    }

    Path Path::operator+(const char* line) const
    {
        Path path = *this;
        return path += line;
    }

    Path Path::operator+(const String& line) const
    {
        Path path = *this;
        return path += line;
    }

    Path& Path::operator/=(const Path& path)
    {
        _M_path.push_back(separator);
        _M_path += path._M_path;
        return simplify();
    }

    Path Path::operator/(const Path& path) const
    {
        Path result = *this;
        return result /= path;
    }

    Path::operator const String&() const
    {
        return _M_path;
    }

    Path::operator const char*() const
    {
        return _M_path.c_str();
    }

    bool Path::operator==(const Path& path) const
    {
        return _M_path == path._M_path;
    }

    bool Path::operator!=(const Path& path) const
    {
        return _M_path != path._M_path;
    }

    bool Path::operator<(const Path& path) const
    {
        return _M_path < path._M_path;
    }

    bool Path::operator<=(const Path& path) const
    {
        return _M_path <= path._M_path;
    }

    bool Path::operator>(const Path& path) const
    {
        return _M_path > path._M_path;
    }

    bool Path::operator>=(const Path& path) const
    {
        return _M_path >= path._M_path;
    }


    Path& Path::simplify()
    {
#if PLATFORM_WINDOWS
        std::replace_if(
                _M_path.begin(), _M_path.end(), [](char c) { return c == '/'; }, Path::separator);
#endif

        // Simplify path
        Vector<String> names;
        Index i                         = 0;
        size_t len                      = _M_path.length();
        size_t first_separator_position = 0;
        bool has_separators             = false;

        while (i < len)
        {
            if (_M_path[i] == Path::separator)
            {
                if (has_separators == false)
                {
                    first_separator_position = i;
                    has_separators           = true;
                }

                while (i < len && _M_path[i] == Path::separator) ++i;
                Index st = i;

                while (i < _M_path.size() && _M_path[i] != Path::separator) ++i;
                if (i == st)
                    break;

                String p = _M_path.substr(st, i - st);

                if (p == current_dir)
                    continue;
                else if (p == parent_dir)
                {
                    if (!names.empty())
                        names.pop_back();
                }
                else
                    names.push_back(p);
            }
            else
            {
                ++i;
            }
        }

        if (!has_separators)
        {
            return *this;
        }

        {
            Index erase_start_pos = first_separator_position + static_cast<Index>(names.empty());
            if (erase_start_pos < len)
            {
                _M_path.erase(erase_start_pos);
            }
        }

        for (auto& name : names)
        {
            _M_path += Path::separator;
            _M_path += name;
        }

        return *this;
    }

    Index Path::separator_index() const
    {
        auto size = _M_path.size();
        return _M_path.find_last_of(separator, size > 1 ? size - 2 : String::npos);
    }

    Pair<const char*, size_t> Path::find_filename() const
    {
        Pair<const char*, size_t> result = {_M_path.c_str(), _M_path.length()};

        Index sep_index = separator_index();

        if (sep_index != String::npos)
        {
            result.first += (sep_index + 1);
            result.second -= (sep_index + 1);
        }

        Index dot_index = _M_path.find_last_of('.');

        if (dot_index != String::npos && sep_index != String::npos)
        {
            result.second -= (_M_path.length() - dot_index);
        }

        return result;
    }

    Path Path::parent() const
    {
        Index index = separator_index();

        if (index != String::npos)
        {
            return Path(_M_path.substr(0, index));
        }

        return Path();
    }

    Path Path::relative(Path base) const
    {
        return std::filesystem::relative(_M_path, base._M_path).string();
    }


    String Path::filename() const
    {
        auto name = find_filename();
        return String(name.first);
    }

    String Path::extension() const
    {
        auto name = find_filename();
        return String(name.first + name.second);
    }

    String Path::stem() const
    {
        auto name = find_filename();
        return String(name.first, name.second);
    }

    bool Path::empty() const
    {
        return _M_path.empty();
    }

    bool Path::starts_with(const Path& path) const
    {
        return _M_path.starts_with(path._M_path);
    }

    size_t Path::length() const
    {
        return _M_path.length();
    }

    const char* Path::c_str() const
    {
        return _M_path.c_str();
    }

    const String& Path::string() const
    {
        return _M_path;
    }


    class DirectoryIteratorInterface
    {
    public:
        virtual bool is_equal(DirectoryIteratorInterface* other) const = 0;
        virtual const Path& path() const                               = 0;
        virtual DirectoryIteratorInterface& next()                     = 0;
        virtual bool is_end() const                                    = 0;
        virtual DirectoryIteratorInterface* copy() const               = 0;
        virtual FileSystemType type() const                            = 0;

        virtual ~DirectoryIteratorInterface() = default;
    };

    DirectoryIterator::DirectoryIterator(DirectoryIteratorInterface* interface) : _M_interface(interface)
    {}

    DirectoryIterator::DirectoryIterator(const Path& path) : _M_interface(nullptr)
    {
        (*this) = rootfs()->directory_iterator(path);
    }

    DirectoryIterator::DirectoryIterator(const DirectoryIterator& other)
    {
        _M_interface = other._M_interface ? other._M_interface->copy() : nullptr;
    }

    DirectoryIterator::DirectoryIterator(DirectoryIterator&& other)
    {
        _M_interface       = other._M_interface;
        other._M_interface = nullptr;
    }

    DirectoryIterator& DirectoryIterator::operator=(const DirectoryIterator& other)
    {
        if (this == &other)
            return *this;

        if (_M_interface)
        {
            delete _M_interface;
            _M_interface = nullptr;
        }

        _M_interface = other._M_interface ? other._M_interface->copy() : nullptr;
        return *this;
    }

    DirectoryIterator& DirectoryIterator::operator=(DirectoryIterator&& other)
    {
        if (this == &other)
            return *this;

        if (_M_interface)
        {
            delete _M_interface;
        }

        _M_interface       = other._M_interface;
        other._M_interface = nullptr;
        return *this;
    }

    DirectoryIterator::~DirectoryIterator()
    {
        if (_M_interface)
        {
            delete _M_interface;
            _M_interface = nullptr;
        }
    }

    DirectoryIterator& DirectoryIterator::begin()
    {
        return *this;
    }

    DirectoryIterator& DirectoryIterator::end()
    {
        static DirectoryIterator it(nullptr);
        return it;
    }

    DirectoryIterator& DirectoryIterator::operator++()
    {
        if (_M_interface)
        {
            _M_interface->next();
        }
        return *this;
    }

    const Path& DirectoryIterator::operator*()
    {
        static const Path path;
        if (_M_interface)
        {
            return _M_interface->path();
        }
        return path;
    }

    bool DirectoryIterator::operator==(const DirectoryIterator& other) const
    {
        if (_M_interface == other._M_interface)
            return true;

        if (_M_interface && other._M_interface)
        {
            return _M_interface->type() == other._M_interface->type() && _M_interface->is_equal(other._M_interface);
        }

        if (_M_interface)
        {
            return _M_interface->is_end();
        }
        else
        {
            return other._M_interface->is_end();
        }
    }

    bool DirectoryIterator::operator!=(const DirectoryIterator& other) const
    {
        return !(*this == other);
    }

    FileSystemInterface::FileSystemInterface(const Path& path) : _M_path(path)
    {}

    const Path& FileSystemInterface::path() const
    {
        return _M_path;
    }

    DirectoryIterator FileSystemInterface::directory_iterator(const Path& path) const
    {
        return DirectoryIterator(nullptr);
    }

    ObjectType FileSystemInterface::type_of(const Path& path) const
    {
        return ObjectType::Undefined;
    }

    bool FileSystemInterface::is_directory(const Path& path) const
    {
        return type_of(path) == ObjectType::Directory;
    }

    bool FileSystemInterface::is_file(const Path& path) const
    {
        return type_of(path) == ObjectType::File;
    }

    bool FileSystemInterface::is_virtual_directory(const Path& path) const
    {
        return type_of(path) == ObjectType::VirtualDirectory;
    }

    bool FileSystemInterface::is_virtual_file(const Path& path) const
    {
        return type_of(path) == ObjectType::VirtualFile;
    }

    bool FileSystemInterface::is_busy() const
    {
        return true;
    }

    FileSystemInterface::~FileSystemInterface()
    {}

    //////////////////////// NATIVE FILE SYSTEM IMPLEMENTATION ////////////////////////


    class NativeDirectoryIterator : public DirectoryIteratorInterface
    {
    private:
        Path _M_path;
        std::filesystem::directory_iterator _M_it;

    public:
        NativeDirectoryIterator() = default;

        NativeDirectoryIterator(const Path& path) : _M_it(std::filesystem::directory_iterator(path.string()))
        {}

        bool is_equal(DirectoryIteratorInterface* other) const override
        {
            return _M_path == reinterpret_cast<NativeDirectoryIterator*>(other)->_M_path;
        }

        const Path& path() const override
        {
            return _M_path;
        }

        DirectoryIteratorInterface& next() override
        {
            ++_M_it;

            if (!is_end())
            {
                _M_path = _M_it->path().string();
            }

            return *this;
        }

        bool is_end() const override
        {
            static std::filesystem::directory_iterator _M_end;
            return _M_it == _M_end;
        }

        DirectoryIteratorInterface* copy() const override
        {
            NativeDirectoryIterator* it = new NativeDirectoryIterator();
            it->_M_it                   = _M_it;
            it->_M_path                 = _M_path;
            return it;
        }

        FileSystemType type() const override
        {
            return FileSystemType::Native;
        }
    };

    class NativeFileSystem : public FileSystemInterface
    {
    private:
        Path _M_native;

    public:
        NativeFileSystem(const Path& virtual_path, const Path& native_path)
            : FileSystemInterface(virtual_path), _M_native(native_path)
        {}

        DirectoryIterator directory_iterator(const Path& path) const override
        {
            DirectoryIteratorInterface* interface = nullptr;
            try
            {
                interface = new NativeDirectoryIterator(_M_native / path);
            }
            catch (...)
            {}

            return DirectoryIterator(interface);
        }

        FileSystemType type() const override
        {
            return FileSystemType::Native;
        }

        FORCE_INLINE std::filesystem::path full_path_of(const Path& path) const
        {
            return std::filesystem::path((_M_native / path).string());
        }

        ObjectType type_of(const Path& path) const override
        {
            const std::filesystem::path& full_path = full_path_of(path);
            if (std::filesystem::is_directory(full_path))
            {
                return ObjectType::Directory;
            }
            else if (std::filesystem::is_regular_file(full_path))
            {
                return ObjectType::File;
            }

            return ObjectType::Undefined;
        }

        bool create_directory(const Path& path) override
        {
            return std::filesystem::create_directories((_M_native / path).string());
        }

        size_t file_size(const Path& path) const override
        {
            return std::filesystem::file_size((_M_native / path).string());
        }
    };


    //////////////////////// VIRTUAL FILE SYSTEM IMPLEMENTATION ////////////////////////

    class VirtualFileSystem : public FileSystemInterface
    {
    public:
        FileSystemType type() const override
        {
            return FileSystemType::Virtual;
        }
    };

    //////////////////////// ROOT FILE SYSTEM IMPLEMENTATION ////////////////////////

    struct MountedFileSystem final {
        Map<String, MountedFileSystem*> next;
        FileSystemInterface* fs   = nullptr;
        MountedFileSystem* parent = nullptr;
        String name;
        bool is_root = false;


        static FORCE_INLINE bool next_name_of(const Path& path, Index& start_index, String& out)
        {
            const String& name = path.string();
            auto _start        = start_index;
            auto it            = name.find_first_of(Path::separator, start_index);

            String::size_type len = 0;
            if (it != String::npos)
            {
                len         = (it - start_index) + static_cast<String::size_type>(start_index == it);
                start_index = it;
            }
            else
            {
                len         = (name.length() - start_index);
                start_index = String::npos;
            }

            if (len > 0)
            {
                out = name.substr(_start, len);
                return true;
            }
            else
            {
                return false;
            }
        }


        MountedFileSystem* create_entry(const Path& path)
        {
            Index index = 0;
            String name;
            MountedFileSystem* current = this;

            while (current && index != String::npos && next_name_of(path, index, name))
            {
                MountedFileSystem* next = nullptr;
                {
                    auto it = current->next.find(name);
                    if (it != current->next.end())
                    {
                        next = it->second;
                    }
                }

                if (next == nullptr)
                {
                    next                = new MountedFileSystem();
                    next->parent        = current;
                    current->next[name] = next;
                    next->name          = name;
                }

                current = next;
                if (index != String::npos)
                    ++index;
            }

            if (current && current->is_root)
            {
                current = nullptr;
            }

            return current;
        }

        MountedFileSystem* find_entry(const Path& path, bool return_last = false)
        {
            Index index = 0;
            String name;
            MountedFileSystem* current = this;

            while (current && index != String::npos && next_name_of(path, index, name))
            {
                MountedFileSystem* next = nullptr;
                {
                    auto it = current->next.find(name);
                    if (it != current->next.end())
                    {
                        next = it->second;
                    }
                }

                if (next == nullptr && return_last)
                {
                    break;
                }

                current = next;
                if (index != String::npos)
                    ++index;
            }

            if (current && current->is_root)
            {
                current = nullptr;
            }

            return current;
        }

        FileSystemInterface* find(const Path& path)
        {
            MountedFileSystem* entry = find_entry(path);
            return entry ? entry->fs : nullptr;
        }

        FileSystemInterface* find_last(const Path& path)
        {
            MountedFileSystem* entry = find_entry(path, true);
            return entry ? entry->fs : nullptr;
        }

        MountedFileSystem& push(const Path& path, FileSystemInterface* interface)
        {
            create_entry(path)->fs = interface;
            return *this;
        }

        bool remove(FileSystemInterface* interface)
        {
            auto entry = find_entry(interface->path());

            if (entry && entry->fs == interface)
            {
                if (entry->next.empty())
                {
                    if (entry->parent)
                    {
                        entry->parent->next.erase(entry->name);
                    }

                    delete entry;
                }
                else
                {
                    entry->fs = nullptr;
                }

                return true;
            }

            return false;
        }

        ~MountedFileSystem()
        {
            for (auto& [name, entry] : next)
            {
                delete entry;
            }

            next.clear();
            parent = nullptr;
        }
    };

    RootFS::RootFS() : FileSystemInterface({}), _M_mounted(new MountedFileSystem())
    {
        _M_mounted->is_root = true;
    }

    DirectoryIterator RootFS::directory_iterator(const Path& path) const
    {
        FileSystemInterface* fs = find_filesystem(path);

        if (fs)
        {
            return fs->directory_iterator(path.relative(fs->path()));
        }

        return FileSystemInterface::directory_iterator(path);
    }

    FileSystemType RootFS::type() const
    {
        return FileSystemType::Virtual;
    }

    ObjectType RootFS::type_of(const Path& path) const
    {
        FileSystemInterface* fs = find_filesystem(path);
        if (fs)
        {
            return fs->type_of(path.relative(fs->path()));
        }

        return ObjectType::Undefined;
    }

    FileSystemInterface* RootFS::mount(const Path& mount, const Path& native)
    {
        if (_M_mounted->find(mount))
        {
            vfs_error("Cannot create mount point '%s'. Point already exist!", mount.c_str());
            return nullptr;
        }

        FileSystemInterface* fs = nullptr;

        if (std::filesystem::is_directory(native.string()))
        {
            fs = new NativeFileSystem(mount, native);
        }
        else if (std::filesystem::is_regular_file(native.string()) &&
                 native.extension() == Constants::virtual_file_system_extension)
        {
        }

        if (fs == nullptr)
        {
            vfs_error("Native path must be native dir or file with '%s' extension",
                      Constants::virtual_file_system_extension.c_str());
        }
        else
        {
            _M_mounted->push(mount, fs);
        }
        return fs;
    }

    bool RootFS::unmount(FileSystemInterface* fs)
    {
        return false;
    }

    FileSystemInterface* RootFS::find_filesystem(const Path& path) const
    {
        FileSystemInterface* fs = _M_mounted->find_last(path);
        if (fs)
        {
            return fs;
        }

        return _M_default;
    }

    RootFS& RootFS::default_fs(FileSystemInterface* fs)
    {
        _M_default = fs;
        return *this;
    }

    FileSystemInterface* RootFS::default_fs() const
    {
        return _M_default;
    }

    bool RootFS::create_directory(const Path& path)
    {
        FileSystemInterface* fs = find_filesystem(path);

        if (fs)
        {
            return fs->create_directory(path.relative(fs->path()));
        }

        return false;
    }

    size_t RootFS::file_size(const Path& path) const
    {
        FileSystemInterface* fs = find_filesystem(path);

        if (fs)
        {
            return fs->create_directory(path.relative(fs->path()));
        }

        return 0;
    }

    RootFS::~RootFS()
    {
        delete _M_mounted;
        _M_mounted = nullptr;
    }

    static RootFS* _M_root_fs = nullptr;

    static void destroy_rootfs()
    {
        if (_M_root_fs)
        {
            delete _M_root_fs;
            _M_root_fs = nullptr;
        }
    }

    ENGINE_EXPORT RootFS* rootfs()
    {
        if (engine_instance->is_shuting_down())
            return nullptr;

        if (!_M_root_fs)
        {
            _M_root_fs = new RootFS();
        }

        return _M_root_fs;
    }

    static PostDestroyController on_destroy(destroy_rootfs);

}// namespace Engine::VFS
