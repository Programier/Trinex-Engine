#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>

namespace Engine
{
    struct ENGINE_EXPORT EngineConfig
    {
        String resources_dir;
        String api;
        String base_commandlet;
        String lua_scripts_dir;

        int_t lz4_compression_level;
        int_t max_gc_collected_objects;

        bool delete_resources_after_load;

        EngineConfig& init(const String& filename);
        EngineConfig& save(const String& filename);


        static EngineConfig& instance();

    private:
        EngineConfig();
    };

    extern ENGINE_EXPORT EngineConfig& engine_config;
}
