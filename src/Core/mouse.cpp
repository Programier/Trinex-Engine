#include <Core/engine_loading_controllers.hpp>
#include <Core/mouse.hpp>
#include <Core/reflection/enum.hpp>

namespace Engine::Mouse
{
	implement_enum(Engine::Mouse::Status, Released, JustReleased, JustPressed, Pressed);
	implement_enum(Engine::Mouse::Button, Button::Left, Button::Middle, Button::Right, Button::Back, Button::Forward);
}// namespace Engine::Mouse
