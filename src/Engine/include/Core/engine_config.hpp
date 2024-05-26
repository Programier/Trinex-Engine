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

        EngineConfig& init();
        virtual EngineConfig& update() override;
    };

    extern ENGINE_EXPORT EngineConfig engine_config;
}// namespace Engine
