#pragma once

namespace Engine
{
	class Sampler;
	class Texture2D;

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

		Texture2D* find_imgui_icon(class Object* object);
	}// namespace Icons
}// namespace Engine
