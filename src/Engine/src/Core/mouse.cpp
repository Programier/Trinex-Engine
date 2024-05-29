#include <Core/engine_loading_controllers.hpp>
#include <Core/mouse.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine::Mouse
{
    static void on_init()
    {
        ScriptEnumRegistrar status_reg("Engine::Mouse::Status");
        ScriptEnumRegistrar button_reg("Engine::Mouse::Button");
        ScriptEnumRegistrar direction_reg("Engine::Mouse::Direction");

        status_reg.set("Released", Released);
        status_reg.set("JustReleased", JustReleased);
        status_reg.set("JustPressed", JustPressed);
        status_reg.set("Pressed", Pressed);

        button_reg.set("Left", Left);
        button_reg.set("Middle", Middle);
        button_reg.set("Right", Right);
        button_reg.set("X1", X1);
        button_reg.set("X2", X2);
        button_reg.set("__COUNT__", __COUNT__);

        direction_reg.set("None", None);
        direction_reg.set("Normal", Normal);
        direction_reg.set("Flipped", Flipped);
    }

    static ReflectionInitializeController controller(on_init, "Bind Mouse");
}// namespace Engine::Mouse
