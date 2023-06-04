#pragma once

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


        template<typename Instance, typename Parent = void>
        static auto lua_class_of(const String& class_name)
        {
            String out_name;
            auto _n                        = namespace_of(class_name, &out_name);
            sol::usertype<Instance> result = _n.new_usertype<Instance>(out_name);
            if constexpr (!std::is_same<Parent, void>::value)
            {
                result.set(sol::base_classes, sol::bases<Parent>());
            }
            return result;
        }

        friend class Engine::EngineInstance;
    };
}// namespace Engine::Lua
