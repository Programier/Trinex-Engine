#include <Core/engine_loading_controllers.hpp>


namespace Engine
{
    static void on_init()
    {
        ReflectionInitializeController()
                .require("Bind Engine Types")
                .require("Bind Engine::Matrix")
                .require("Bind Engine::Quaternion")
                .require("Bind Engine::BoolVector")
                .require("Bind Engine::Vector")
                .require("Bind Engine::IntVector")
                .require("Bind Engine::UIntVector")
                .require("Bind Engine::StringView")
                .require("Bind ImGui")
                .require("Bind Engine::Name")
                .require("Bind Engine::Transform")
                .require("Bind Event")
                .require("Bind EventData")
                .require("Bind Engine::Class");
    }

    static ReflectionInitializeController bindings(on_init, "Initialize bindings");
}// namespace Engine
