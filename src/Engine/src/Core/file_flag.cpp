#include <Core/archive.hpp>
#include <Core/file_flag.hpp>


namespace Engine
{
    static constexpr size_t make_flag_from_string(const char* string, size_t value = 0)
    {
        return *string == 0 ? value : make_flag_from_string(string + 1, (value << 8) | *string);
    }


    FileFlag::FileFlag(size_t first, size_t second)
    {
        data[0] = first;
        data[1] = second;
    }

    bool FileFlag::operator==(const FileFlag& other) const
    {
        return data[0] == other.data[0] && data[1] == other.data[1];
    }

    bool FileFlag::operator!=(const FileFlag& other) const
    {
        return !(*this == other);
    }

    const FileFlag& FileFlag::package_flag()
    {
        static FileFlag flag(make_flag_from_string("TRINEX"), make_flag_from_string("PACKAGE"));
        return flag;
    }

    ENGINE_EXPORT bool operator&(Archive& ar, FileFlag& flag)
    {
        ar& flag.data[0];
        ar& flag.data[1];

        return ar;
    }
}// namespace Engine
