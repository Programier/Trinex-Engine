#include <Core/engine_loading_controllers.hpp>
#include <Core/enum.hpp>
#include <Core/mouse.hpp>

namespace Engine::Mouse
{
	implement_enum(Status, Engine::Mouse, {"Released", Released}, {"JustReleased", JustReleased}, {"JustPressed", JustPressed},
	               {"Pressed", Pressed});

	implement_enum(Button, Engine::Mouse, {"Left", Button::Left}, {"Middle", Button::Middle}, {"Right", Button::Right},
	               {"Back", Button::Back}, {"Forward", Button::Forward});
}// namespace Engine::Mouse
