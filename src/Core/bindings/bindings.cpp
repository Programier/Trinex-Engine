#include <Core/engine_loading_controllers.hpp>


namespace Engine
{
	static void on_init()
	{
		ReflectionInitializeController()
		        .require("Engine Types")
		        .require("Engine::Matrix")
		        .require("Engine::Quaternion")
		        .require("Engine::BoolVector")
		        .require("Engine::Vector")
		        .require("Engine::IntVector")
		        .require("Engine::UIntVector")
		        .require("Engine::StringView")
		        .require("ImGui")
		        .require("Engine::Name")
		        .require("Engine::Transform")
		        .require("Engine::Event")
		        .require("Engine::EventData")
		        .require("Engine::Class");
	}

	static ReflectionInitializeController bindings(on_init, "Initialize bindings");
}// namespace Engine
