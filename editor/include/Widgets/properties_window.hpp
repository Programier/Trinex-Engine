#pragma once
#include <Core/etl/type_info.hpp>
#include <Core/reflection/object.hpp>
#include <Graphics/imgui.hpp>

namespace Engine
{
	class ImGuiObjectProperties : public ImGuiWidget
	{
	public:
		using PropertiesMap    = TreeMap<String, Vector<Refl::Property*>>;
		using PropertyRenderer = Function<bool(ImGuiObjectProperties* wnd, void* obj, Refl::Property* prop, bool read_only)>;

	private:
		Object* m_object;
		Identifier m_destroy_id;

		TreeMap<Refl::Struct*, PropertiesMap> m_properties;
		PropertiesMap& build_props_map(Refl::Struct* self);


	public:
		CallBacks<void(Object*)> on_begin_render;

		ImGuiObjectProperties();
		~ImGuiObjectProperties();

		bool render(RenderViewport* viewport) override;
		Refl::Struct* struct_instance() const;
		Object* object() const;

		ImGuiObjectProperties& update(Object* object);
		const PropertiesMap& properties_map(Refl::Struct* self);
		static bool collapsing_header(const void* id, const char* format, ...);

		bool render_property(void* object, Refl::Property* prop, bool read_only = false);
		bool render_struct_properties(void* object, class Refl::Struct* struct_class, bool read_only = false);

		virtual const char* name() const;
		static const char* static_name();

		static void register_prop_renderer(const Refl::Object::ReflClassInfo*, const PropertyRenderer& renderer);
		template<typename T>
		static void register_prop_renderer(const PropertyRenderer& renderer)
		{
			register_prop_renderer(T::static_refl_class_info(), renderer);
		}
	};

}// namespace Engine
