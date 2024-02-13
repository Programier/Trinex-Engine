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
        Vector<String> languages;

        Path packages_dir;
        Path scripts_dir;
        Path libraries_dir;
        Path config_dir;
        Path localization_dir;

        String api;
        String window_system;
        String default_language;
        String current_language;
        String shading_language;

        uint_t lz4_compression_level;
        uint_t gc_max_object_per_tick;
        uint_t fps_limit = 60;
        bool enable_jit;

        float gamma;
        float gc_wait_time;
        virtual EngineConfig& update() override;
    };

    extern ENGINE_EXPORT EngineConfig engine_config;
}// namespace Engine
