#pragma once

#include <Core/engine_types.hpp>

namespace Engine
{
    struct ENGINE_EXPORT FileFlag final {
        size_t data[2];

        FileFlag(size_t first = 0, size_t second = 0);

        bool operator==(const FileFlag& other) const;
        bool operator!=(const FileFlag& other) const;


        static const FileFlag& package_flag();
    };

    class Archive;
    ENGINE_EXPORT bool operator&(Archive& ar, FileFlag& flag);
}// namespace Engine
