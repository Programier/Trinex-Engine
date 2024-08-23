#pragma once
#include <Graphics/imgui.hpp>

namespace Engine
{
	class ImGuiObjectProperties : public ImGuiWidget
	{
	public:
		using PropertiesMap = TreeMap<Name, Vector<class Property*>>;

	private:
		Object* m_object;
		Identifier m_destroy_id;

		TreeMap<class Struct*, PropertiesMap> m_properties;
		PropertiesMap& build_props_map(Struct* self);

	public:
		size_t row_index = 0;
		CallBacks<void(Object*)> on_begin_render;

		ImGuiObjectProperties();
		~ImGuiObjectProperties();

		bool render(RenderViewport* viewport) override;
		Struct* struct_instance() const;
		Object* object() const;

		ImGuiObjectProperties& update(Object* object);
		const PropertiesMap& properties_map(Struct* self);
		ImGuiObjectProperties& render_struct_properties(void* object, class Struct* struct_class, bool editable = true);
		ImGuiObjectProperties& setup_next_row();
		static bool collapsing_header(const void* id, const char* format, ...);
		
		virtual const char* name() const;
		static const char* static_name();
	};

}// namespace Engine
