#pragma once
#include <Core/build.hpp>
#include <Core/config.hpp>
#include <Core/engine_types.hpp>

namespace Engine
{
    struct ENGINE_EXPORT EngineConfig : public Config {
#if TRINEX_WITH_SKIP_JIT_INSTRUCTIONS
        Map<String, Vector<int>> jit_skip_instructions;
#endif

        Vector<String> external_system_libraries;
        Vector<String> systems;

        Path resources_dir;
        Path scripts_dir;
        Path libraries_dir;
        Path config_dir;

        String api;
        String shader_compilers_lib;
        String shader_compiler;
        String window_system;

        uint_t lz4_compression_level;
        uint_t max_gc_collected_objects;
        uint_t fps_limit = 60;
        bool enable_jit;
        virtual EngineConfig& update() override;
    };

    extern ENGINE_EXPORT EngineConfig engine_config;
}// namespace Engine
