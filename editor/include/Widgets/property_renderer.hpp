#pragma once
#include <Core/etl/type_info.hpp>
#include <Graphics/imgui.hpp>

namespace Engine
{
	namespace Refl
	{
		struct ClassInfo;
	}

	class ScriptFunction;
	class PropertyRenderer : public ImGuiWidget
	{
	public:
		using PropertiesMap = TreeMap<String, Vector<Refl::Property*>>;
		using RendererFunc  = Function<bool(PropertyRenderer* wnd, void* obj, Refl::Property* prop, bool read_only)>;

	private:
		String m_next_prop_name;
		Object* m_object;
		Identifier m_destroy_id;
		bool m_is_property_skipped;

		TreeMap<Refl::Struct*, PropertiesMap> m_properties;
		PropertiesMap& build_props_map(Refl::Struct* self);


	public:
		CallBacks<void(Object*)> on_begin_render;

		PropertyRenderer();
		~PropertyRenderer();

		bool render(RenderViewport* viewport) override;
		Refl::Struct* struct_instance() const;
		Object* object() const;

		PropertyRenderer& update(Object* object);
		const PropertiesMap& properties_map(Refl::Struct* self);

		bool collapsing_header(Refl::Property* prop);
		static bool collapsing_header(const void* id, const char* format, ...);

		void mark_property_skipped();
		bool is_property_skipped() const;
		void create_row();
		void next_prop_name(const String& name);
		const String& next_prop_name() const;
		void render_name(Refl::Property* prop);
		bool render_property(void* object, Refl::Property* prop, bool read_only = false, bool allow_script_call = true);
		bool render_struct_properties(void* object, class Refl::Struct* struct_class, bool read_only = false);

		virtual const char* name() const;
		static const char* static_name();

		static void register_prop_renderer(const Refl::ClassInfo*, const RendererFunc& renderer);
		template<typename T>
		static void register_prop_renderer(const RendererFunc& renderer)
		{
			register_prop_renderer(T::static_refl_class_info(), renderer);
		}
	};

}// namespace Engine
