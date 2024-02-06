#pragma once
#include <Core/export.hpp>

namespace Engine::VFS
{
    class Path;

    class ENGINE_EXPORT DirectoryIteratorInterface
    {
    protected:
        enum Type
        {
            Virtual = 1,
            Native  = 2,
        };

        virtual void next()                                = 0;
        virtual const Path& path()                         = 0;
        virtual bool is_valid() const                      = 0;
        virtual DirectoryIteratorInterface* copy()         = 0;
        virtual Type type() const                          = 0;
        virtual bool is_equal(DirectoryIteratorInterface*) = 0;
        virtual ~DirectoryIteratorInterface()              = default;

    public:
        friend class DirectoryIterator;
    };

    class ENGINE_EXPORT DirectoryIterator final
    {
    private:
        DirectoryIteratorInterface* _M_interface;


    public:
        DirectoryIterator();
        DirectoryIterator(const Path& path);
        DirectoryIterator(const DirectoryIterator&);
        DirectoryIterator(DirectoryIterator&&);

        DirectoryIterator& operator=(const DirectoryIterator&);
        DirectoryIterator& operator=(DirectoryIterator&&);

        DirectoryIterator& begin();
        DirectoryIterator& end();
        DirectoryIterator& operator++();
        const Path& operator*();

        bool operator!=(const DirectoryIterator& other) const;
        bool operator==(const DirectoryIterator& other) const;
    };
}// namespace Engine::VFS
