#pragma once
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>

namespace Engine
{
	class Object;

	extern Map<Refl::Struct*, void (*)(class PropertyRenderer*, void*, Refl::Struct*, bool)> special_class_properties_renderers;
}// namespace Engine
