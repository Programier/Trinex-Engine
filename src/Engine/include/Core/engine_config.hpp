#pragma once
#include <Core/config.hpp>
#include <Core/engine_types.hpp>

namespace Engine
{
    struct ENGINE_EXPORT EngineConfig : public Config {
        Vector<String> external_system_libraries;
        Vector<String> systems;

        String resources_dir;
        String api;
        String lua_scripts_dir;
        String libraries_dir;
        String shader_compilers_lib;
        String shader_compiler;
        String window_system;

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


        virtual EngineConfig& update() override;
    };

    extern ENGINE_EXPORT EngineConfig engine_config;
}// namespace Engine
