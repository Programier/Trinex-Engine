#include <Core/editor_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/icons.hpp>
#include <Core/object.hpp>
#include <Core/package.hpp>
#include <Core/pointer.hpp>
#include <Core/reflection/class.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine::Icons
{
	static Texture2D* m_icons[IconType::__COUNT__] = {0};

	ImGuiTrinexTextureId default_texture()
	{
		return m_icons[IconType::Default];
	}

	ImGuiTrinexTextureId icon(IconType type)
	{
		return m_icons[type];
	}

	ImGuiTrinexTextureId find_icon(Object* object)
	{
		if (object)
		{
			if (Texture2D* texture = object->instance_cast<Texture2D>())
			{
				return texture;
			}

			if (RenderSurface* surface = object->instance_cast<RenderSurface>())
				return surface;

			return find_icon(object->class_instance());
		}

		return default_texture();
	}

	ImGuiTrinexTextureId find_icon(Refl::Class* class_instance)
	{
		static Name meta = "__trinex_editor_class_icon__";
		auto& data       = class_instance->metadata(meta);

		if (data.has_value())
			return data.cast<const Pointer<Texture2D>&>().ptr();

		return default_texture();
	}

	void on_editor_package_loaded()
	{
		m_icons[IconType::Default] = EditorResources::default_icon;
		m_icons[IconType::Add]     = EditorResources::add_icon;
		m_icons[IconType::Remove]  = EditorResources::remove_icon;
		m_icons[IconType::Select]  = EditorResources::select_icon;
		m_icons[IconType::Move]    = EditorResources::move_icon;
		m_icons[IconType::Rotate]  = EditorResources::rotate_icon;
		m_icons[IconType::Scale]   = EditorResources::scale_icon;
		m_icons[IconType::More]    = EditorResources::more_icon;

		for (Texture2D* icon : m_icons)
		{
			trinex_always_check(icon, "Icon can't be null!");
		}
	}
}// namespace Engine::Icons
