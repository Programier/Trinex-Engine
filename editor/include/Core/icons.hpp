#pragma once

namespace Engine
{
	class Sampler;
	class Texture2D;
	class Object;

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
		Sampler* default_sampler();
		Texture2D* default_texture();

		Texture2D* find_imgui_icon(Object* object);
	}// namespace Icons
}// namespace Engine
