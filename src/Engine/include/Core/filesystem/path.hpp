#pragma once
#include <Core/engine_types.hpp>

namespace Engine::VFS
{
    class ENGINE_EXPORT Path final
    {
    private:
        String _M_path;

        StringView _M_extension;
        StringView _M_base_name;
        StringView _M_stem;
        StringView _M_base_path;

        Path& on_path_changed();

    public:
        static const char separator;

        Path();
        Path(const Path&);
        Path(const Path&&);
        Path(const StringView& path);
        Path(const char*);
        Path(const String&);

        Path& operator=(const Path&);
        Path& operator=(Path&&);
        Path& operator=(const StringView& path);
        Path& operator=(const String& path);
        Path& operator=(const char* path);
        Path& operator/=(const Path& path);

        FORCE_INLINE Path operator/(const Path& path) const
        {
            Path result = *this;
            return result /= path;
        }

        FORCE_INLINE const String& path() const
        {
            return _M_path;
        }

        FORCE_INLINE const StringView& extension() const
        {
            return _M_extension;
        }

        FORCE_INLINE const StringView& base_name() const
        {
            return _M_base_name;
        }

        FORCE_INLINE const StringView& stem() const
        {
            return _M_stem;
        }

        FORCE_INLINE const char* c_str() const
        {
            return _M_path.c_str();
        }

        FORCE_INLINE const String& str() const
        {
            return _M_path;
        }

        FORCE_INLINE operator const String&() const
        {
            return str();
        }

        FORCE_INLINE operator const char*() const
        {
            return c_str();
        }

        FORCE_INLINE const StringView& base_path() const
        {
            return _M_base_path;
        }

        FORCE_INLINE size_t length() const
        {
            return _M_path.length();
        }

        FORCE_INLINE bool is_empty() const
        {
            return length() == 0;
        }

        FORCE_INLINE bool operator==(const Path& path) const
        {
            return _M_path == path._M_path;
        }

        FORCE_INLINE bool operator!=(const Path& path) const
        {
            return _M_path != path._M_path;
        }
    };
}// namespace Engine::VFS
