#include <Core/filesystem.hpp>


namespace Engine::FileSystem
{
    ENGINE_EXPORT String dirname_of(const String& filename)
    {
        auto index = filename.find_last_of("/\\");
        if (index == String::npos)
        {
            return STR("./");
        }

        return filename.substr(0, index) + STR("/");
    }

    ENGINE_EXPORT String basename_of(const String& filename)
    {
        auto index = filename.find_last_of("/\\") + 1;
        return filename.substr(index, filename.length() - index);
    }

}// namespace Engine::FileSystem
