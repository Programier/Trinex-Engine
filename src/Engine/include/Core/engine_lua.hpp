#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <LuaJIT/lua.hpp>

#include <LuaBridge/LuaBridge.h>

namespace Engine
{

    using LuaResult = Vector<luabridge::LuaRef>;
    struct ENGINE_EXPORT LuaInterpretter {
    private:
        ENGINE_EXPORT static void init();
        ENGINE_EXPORT static void terminate();

    public:

        ENGINE_EXPORT static LuaResult execute_string(const char* line);
        ENGINE_EXPORT static LuaResult execute_string(const String& line);
        friend class EngineInstance;
    };
}// namespace Engine
