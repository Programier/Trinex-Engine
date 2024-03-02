#pragma once

#include <Core/engine_types.hpp>

namespace Engine
{
    struct ENGINE_EXPORT FileFlag final {
        union
        {
            size_t data[2];
            char line[8 * (sizeof(data) / sizeof(data[0]))];
        };

        FileFlag(size_t first = 0, size_t second = 0);

        bool operator==(const FileFlag& other) const;
        bool operator!=(const FileFlag& other) const;


        static const FileFlag& package_flag();
        static const FileFlag& asset_flag();
    };

    class Archive;
    ENGINE_EXPORT bool operator&(Archive& ar, FileFlag& flag);
}// namespace Engine
