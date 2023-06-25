#pragma once

// Always use release configuration for sol2
#define SOL_IN_DEBUG_DETECTED 0

#include <Core/engine_types.hpp>
#include <Core/logger.hpp>
#include <sol/sol.hpp>


namespace Engine
{
    class EngineInstance;
}

namespace Engine::Lua
{
    using namespace sol;


    using Result    = protected_function_result;
    using Namespace = table;

    template<typename Instance>
    using Class = usertype<Instance>;

    struct ENGINE_EXPORT Interpretter {
    private:
        ENGINE_EXPORT static void init();
        ENGINE_EXPORT static void init_lua_dir();
        ENGINE_EXPORT static void terminate();


    public:
        ENGINE_EXPORT static Result execute_string(const char* line);
        ENGINE_EXPORT static Result execute_string(const String& line);
        ENGINE_EXPORT static sol::global_table& global_namespace();
        ENGINE_EXPORT static sol::state* state();
        ENGINE_EXPORT static Namespace namespace_of(const String& name, String* class_name = nullptr);


        template<typename Instance>
        static Class<Instance> lua_class_of(const String& class_name)
        {
            String out_name;
            return namespace_of(class_name, &out_name).new_usertype<Instance>(out_name);
        }

        friend class Engine::EngineInstance;
    };
}// namespace Engine::Lua
