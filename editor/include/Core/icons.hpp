#pragma once

class ImGuiTrinexTextureId;

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
		enum IconType
		{
			Default,
			Add,
			Remove,
			Select,
			Move,
			Rotate,
			Scale,
			More,
			__COUNT__
		};

		ImGuiTrinexTextureId icon(IconType type);
		ImGuiTrinexTextureId default_texture();

		ImGuiTrinexTextureId find_icon(Object* object);
		ImGuiTrinexTextureId find_icon(Refl::Class* class_instance);
	}// namespace Icons
}// namespace Engine
