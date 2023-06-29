#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>

namespace Engine
{
    struct ENGINE_EXPORT EngineConfig {
        String resources_dir;
        String api;
        String base_commandlet;
        String lua_scripts_dir;

        uint_t lz4_compression_level;
        uint_t max_gc_collected_objects;
        uint_t min_g_buffer_width;
        uint_t min_g_buffer_height;
        uint_t max_g_buffer_width;
        uint_t max_g_buffer_height;
        uint_t back_buffer_count;

        bool delete_resources_after_load;
        bool load_shaders_to_gpu;
        bool load_meshes_to_gpu;
        bool load_textures_to_gpu;
        bool enable_g_buffer;

        EngineConfig& init(const String& filename);
        EngineConfig& save(const String& filename);


        static EngineConfig& instance();
        EngineConfig& init_callback(void (*)(EngineConfig*));

    private:
        EngineConfig();
        void (*_M_callback)(EngineConfig*) = nullptr;
    };

    extern ENGINE_EXPORT EngineConfig& engine_config;
}// namespace Engine
