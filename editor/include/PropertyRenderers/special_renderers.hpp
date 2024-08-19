#pragma once
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>

namespace Engine
{
	class Struct;
	class Object;

	extern Map<Struct*, void (*)(class ImGuiObjectProperties*, void*, Struct*, bool)> special_class_properties_renderers;
}// namespace Engine
