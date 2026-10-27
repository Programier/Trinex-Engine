#pragma once

struct ImGuiTrinexTextureId;

namespace Trinex
{
	class Texture2D;
	class DeprecatedObject;

	namespace Refl
	{
		class Class;
	}

	namespace Icons
	{
		ImGuiTrinexTextureId default_texture();

		ImGuiTrinexTextureId find_icon(DeprecatedObject* object);
		ImGuiTrinexTextureId find_icon(Refl::Class* class_instance);
	} // namespace Icons
} // namespace Trinex
