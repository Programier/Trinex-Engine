#pragma once

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

		Texture2D* icon(IconType type);
		Texture2D* default_texture();

		Texture2D* find_icon(Object* object);
		Texture2D* find_icon(Refl::Class* class_instance);
	}// namespace Icons
}// namespace Engine
