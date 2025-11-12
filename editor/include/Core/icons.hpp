#pragma once

struct ImGuiTrinexTextureId;

namespace Engine
{
	class Texture2D;
	class Object;

	namespace Refl
	{
		class Class;
	}

	namespace Icons
	{
		ImGuiTrinexTextureId default_texture();

		ImGuiTrinexTextureId find_icon(Object* object);
		ImGuiTrinexTextureId find_icon(Refl::Class* class_instance);
	}// namespace Icons
}// namespace Engine
