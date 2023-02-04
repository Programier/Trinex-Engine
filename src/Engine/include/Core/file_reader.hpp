#pragma once

#include <Core/export.hpp>
#include <Core/engine_types.hpp>
#include <fstream>
#include <Core/implement.hpp>
#include <Core/constants.hpp>


namespace Engine
{
    ENGINE_EXPORT class FileReader
    {
    public:
        using read_pos = std::size_t;

    private:
        std::ifstream _M_file;
        size_t _M_size = 0;
        read_pos _M_position = 0;

        size_t max_size(size_t size);

    public:
        delete_copy_constructors(FileReader);
        FileReader();
        FileReader(const String& filename);
        FileReader& open(const String& filename);
        FileReader& close();
        bool is_open() const;
        std::size_t size();
        read_pos position() const;
        FileReader& position(read_pos pos);

        bool read(byte* to, size_t size);
        bool read(FileBuffer& buffer, size_t size = Constants::max_size);

        template<typename Type>
        bool read(Type& value)
        {
            return read(reinterpret_cast<byte*>(&value), sizeof(value));
        }
    };
}
