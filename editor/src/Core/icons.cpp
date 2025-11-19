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
#include <Graphics/texture.hpp>

namespace Engine::Icons
{
	ImGuiTrinexTextureId default_texture()
	{
		return ImGuiTrinexTextureId(EditorResources::default_icon->rhi_texture());
	}

	ImGuiTrinexTextureId find_icon(Object* object)
	{
		if (object)
		{
			if (Texture2D* texture = object->instance_cast<Texture2D>())
			{
				return texture->rhi_texture();
			}

			if (RenderSurface* surface = object->instance_cast<RenderSurface>())
				return surface->rhi_texture();

			return find_icon(object->class_instance());
		}

		return default_texture();
	}

	ImGuiTrinexTextureId find_icon(Refl::Class* class_instance)
	{
		static Name meta = "__trinex_editor_class_icon__";
		auto& data       = class_instance->metadata(meta);

		if (data.has_value())
			return data.cast<const Pointer<Texture2D>&>().ptr()->rhi_texture();

		return default_texture();
	}
}// namespace Engine::Icons
