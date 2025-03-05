#include <Core/engine_loading_controllers.hpp>
#include <Core/mouse.hpp>
#include <Core/reflection/enum.hpp>

namespace Engine::Mouse
{
	trinex_implement_engine_enum(Mouse::Status, Released, JustReleased, JustPressed, Pressed);
	trinex_implement_engine_enum(Mouse::Button, Left, Middle, Right, Back, Forward);
}// namespace Engine::Mouse
