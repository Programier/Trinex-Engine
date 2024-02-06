#include <Core/exception.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/string_functions.hpp>

namespace Engine::VFS
{
    const char Path::separator = '/';

    static FORCE_INLINE void simplify_path(String& path)
    {
        static auto simplify_separators = [](char a, char b) { return a == b && a == Path::separator; };
        auto end                        = std::unique(path.begin(), path.end(), simplify_separators);
        path.erase(end, path.end());

        if (path.length() > 1 && path.back() == Path::separator)
            path.pop_back();
    }

    Path& Path::on_path_changed()
    {
#if PLATFORM_WINDOWS
        static auto transform_func = [](char ch) -> char {
            if (ch == '\\')
                return '/';
            return ch;
        };

        std::transform(_M_path.begin(), _M_path.end(), _M_path.begin(), transform_func);
#endif

        simplify_path(_M_path);


        StringView view = _M_path;

        Index position;

        {
            position = view.rfind(separator);
            if (position != StringView::npos)
            {
                _M_base_name = view.substr(position + 1);
                _M_base_path = view.substr(0, position);
            }
            else
            {
                _M_base_name = {};
                _M_base_path = view;
            }
        }

        {
            position = _M_base_name.rfind('.');

            if (position != StringView::npos)
            {
                _M_extension = _M_base_name.substr(position);
                _M_stem      = _M_base_name.substr(0, position);
            }
            else
            {
                _M_extension = {};
                _M_stem      = _M_base_name;
            }
        }

        return *this;
    }

    Path::Path()
    {}

    Path::Path(const Path& path) : _M_path(path._M_path)
    {
        on_path_changed();
    }

    Path::Path(const Path&& path) : _M_path(std::move(path._M_path))
    {
        on_path_changed();
    }

    Path::Path(const StringView& path) : _M_path(path)
    {
        on_path_changed();
    }

    Path::Path(const char* str) : Path(StringView(str))
    {}

    Path::Path(const String& str) : Path(StringView(str))
    {}

    Path& Path::operator=(const Path& path)
    {
        if (this == &path)
            return *this;

        _M_path = path._M_path;
        return on_path_changed();
    }

    Path& Path::operator=(Path&& path)
    {
        if (this == &path)
            return *this;

        _M_path = std::move(path._M_path);
        return on_path_changed();
    }

    Path& Path::operator=(const StringView& path)
    {
        _M_path = String(path);
        return on_path_changed();
    }

    Path& Path::operator=(const String& path)
    {
        return (*this) = StringView(path);
    }

    Path& Path::operator=(const char* path)
    {
        if(path == nullptr)
        {
            new(this) Path();
            return *this;
        }
        return (*this) = StringView(path);
    }

    Path& Path::operator/=(const Path& path)
    {
        if (!is_empty() && _M_path.back() != Path::separator)
        {
            _M_path.push_back(separator);
        }

        _M_path += path._M_path;
        return on_path_changed();
    }
}// namespace Engine::VFS
