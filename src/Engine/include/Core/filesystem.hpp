#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>


// Virtual File System implementation
namespace Engine::VFS
{
    enum class ObjectType
    {
        Undefined        = 0,
        File             = 1,
        Directory        = 2,
        VirtualFile      = 3,
        VirtualDirectory = 4,
    };

    enum class FileSystemType
    {
        Root    = 1,
        Native  = 2,
        Virtual = 3,
    };

    class ENGINE_EXPORT Path final
    {
    private:
        String _M_path;


        Index separator_index() const;
        Pair<const char*, size_t> find_filename() const;

        Path& simplify();

    public:
        static const char separator;

        copy_constructors_hpp(Path);
        Path();
        Path(const String& path);
        Path(String&& path);
        Path(const char*);

        Path& operator=(const String& path);
        Path& operator=(String&& path);
        Path& operator=(const char*);

        Path& operator+=(const char* line);
        Path& operator+=(const String& line);
        Path operator+(const char* line) const;
        Path operator+(const String& line) const;

        Path& operator/=(const Path& path);
        Path operator/(const Path& path) const;

        operator const String&() const;
        operator const char*() const;

        bool operator==(const Path&) const;
        bool operator!=(const Path&) const;
        bool operator<(const Path&) const;
        bool operator<=(const Path&) const;
        bool operator>(const Path&) const;
        bool operator>=(const Path&) const;

        Path parent() const;
        Path relative(Path base) const;
        bool empty() const;
        bool starts_with(const Path& path) const;
        size_t length() const;

        String filename() const;
        String extension() const;
        String stem() const;

        const char* c_str() const;
        const String& string() const;
    };

    class ENGINE_EXPORT DirectoryIterator final
    {
    public:
        DirectoryIterator(class DirectoryIteratorInterface*);
        DirectoryIterator(const Path& path);
        ~DirectoryIterator();

        copy_constructors_hpp(DirectoryIterator);

        DirectoryIterator& begin();
        DirectoryIterator& end();
        DirectoryIterator& operator++();
        const Path& operator*();

        bool operator!=(const DirectoryIterator& other) const;
        bool operator==(const DirectoryIterator& other) const;

    private:
        DirectoryIteratorInterface* _M_interface;
    };


    class ENGINE_EXPORT FileSystemInterface
    {
    protected:
        Path _M_path;

    public:
        FileSystemInterface(const Path& path);
        const Path& path() const;

        bool is_directory(const Path& path) const;
        bool is_file(const Path& path) const;
        bool is_virtual_directory(const Path& path) const;
        bool is_virtual_file(const Path& path) const;

        virtual FileSystemType type() const = 0;
        virtual ObjectType type_of(const Path& path) const;
        virtual DirectoryIterator directory_iterator(const Path& path) const;
        virtual bool is_busy() const;

        virtual bool create_directory(const Path& path)  = 0;
        virtual size_t file_size(const Path& path) const = 0;

        virtual ~FileSystemInterface();
    };


    class ENGINE_EXPORT RootFS : public FileSystemInterface
    {
    private:
        struct MountedFileSystem* _M_mounted;
        FileSystemInterface* _M_default;

    public:
        RootFS();

        DirectoryIterator directory_iterator(const Path& path) const override;
        FileSystemType type() const override;
        ObjectType type_of(const Path& path) const override;

        FileSystemInterface* mount(const Path& mount, const Path& native);
        bool unmount(FileSystemInterface*);
        FileSystemInterface* find_filesystem(const Path& path) const;

        RootFS& default_fs(FileSystemInterface* fs);
        FileSystemInterface* default_fs() const;

        bool create_directory(const Path& path) override;
        size_t file_size(const Path& path) const override;

        ~RootFS();
    };

    ENGINE_EXPORT RootFS* rootfs();
}// namespace Engine::VFS
