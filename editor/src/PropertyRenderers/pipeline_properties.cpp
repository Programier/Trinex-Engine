#include <Graphics/imgui.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <PropertyRenderers/special_renderers.hpp>
#include <Widgets/properties_window.hpp>

namespace Engine
{
	static void renderer(class ImGuiObjectProperties* window, void* object, Refl::Struct* self, bool editable)
	{
		Pipeline* pipeline          = reinterpret_cast<Pipeline*>(object);
		VertexShader* vertex_shader = pipeline->vertex_shader();
		if (vertex_shader == nullptr)
			return;

		window->render_struct_properties(vertex_shader, reinterpret_cast<Refl::Struct*>(vertex_shader->class_instance()),
										 editable);
	}

	static void initialize_special_class_properties_renderers()
	{
		special_class_properties_renderers[reinterpret_cast<Refl::Struct*>(Pipeline::static_class_instance())] = renderer;
	}

	static InitializeController on_post_init(initialize_special_class_properties_renderers);
}// namespace Engine
