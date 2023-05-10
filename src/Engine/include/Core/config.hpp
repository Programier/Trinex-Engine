#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>

namespace Engine
{
    struct ENGINE_EXPORT ConfigStringValue
    {
        String resources_dir;
        String api;
        String base_commandlet;
        ConfigStringValue& init(class TextFileReader* reader);
    };

    struct ENGINE_EXPORT ConfigBooleanValue
    {
        bool delete_resources_after_load;
        ConfigBooleanValue& init(class TextFileReader* reader);
    };

    struct ENGINE_EXPORT ConfigIntegerValue
    {
        int_t lz4_compression_level;
        int_t max_gc_collected_objects;
        ConfigIntegerValue& init(class TextFileReader* reader);
    };

    struct ENGINE_EXPORT EngineConfig : ConfigStringValue, ConfigBooleanValue, ConfigIntegerValue
    {
        EngineConfig& init(const String& filename);
    };

    extern ENGINE_EXPORT EngineConfig engine_config;
}
