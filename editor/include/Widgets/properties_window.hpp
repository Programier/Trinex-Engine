#pragma once
#include <Core/etl/type_info.hpp>
#include <Graphics/imgui.hpp>

namespace Engine
{
	class ImGuiObjectProperties : public ImGuiWidget
	{
	public:
		using PropertiesMap    = TreeMap<Name, Vector<class Property*>>;
		using PropertyRenderer = Function<bool(ImGuiObjectProperties* wnd, void* obj, Property* prop, bool can_edit)>;

	private:
		Object* m_object;
		Identifier m_destroy_id;

		TreeMap<Refl::Struct*, PropertiesMap> m_properties;
		PropertiesMap& build_props_map(Refl::Struct* self);

		static void register_prop_renderer(StringView type_name, const PropertyRenderer& renderer);

	public:
		size_t row_index = 0;
		CallBacks<void(Object*)> on_begin_render;

		ImGuiObjectProperties();
		~ImGuiObjectProperties();

		bool render(RenderViewport* viewport) override;
		Refl::Struct* struct_instance() const;
		Object* object() const;

		ImGuiObjectProperties& update(Object* object);
		const PropertiesMap& properties_map(Refl::Struct* self);
		ImGuiObjectProperties& render_struct_properties(void* object, class Refl::Struct* struct_class, bool editable = true);
		ImGuiObjectProperties& setup_next_row();
		static bool collapsing_header(const void* id, const char* format, ...);

		virtual const char* name() const;
		static const char* static_name();

		template<typename T>
		static void register_prop_renderer(const PropertyRenderer& renderer)
		{
			register_prop_renderer(type_info<T>::name(), renderer);
		}
	};

}// namespace Engine
