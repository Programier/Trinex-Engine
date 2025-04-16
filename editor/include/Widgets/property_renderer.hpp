#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/type_info.hpp>
#include <Core/userdata.hpp>
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
		using RendererFunc  = Function<bool(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)>;

		class Context
		{
		public:
			using BeginRenderingFunc = Function<bool(PropertyRenderer* renderer)>;
			using EndRenderingFunc   = Function<void(PropertyRenderer* renderer, bool)>;
			using BeginGroupFunc     = Function<bool(PropertyRenderer* renderer, const String&)>;
			using EndGroupFunc       = Function<void(PropertyRenderer* renderer, const String&, bool)>;

		private:
			Map<const Refl::ClassInfo*, RendererFunc> m_renderers;
			Context* m_prev = nullptr;

		public:
			BeginRenderingFunc on_begin_rendering;
			EndRenderingFunc on_end_rendering;
			BeginGroupFunc on_begin_group;
			EndGroupFunc on_end_group;

			Context(Context* prev = nullptr) : m_prev(prev) {}

			const RendererFunc& renderer(const Refl::ClassInfo*);
			const RendererFunc* renderer_ptr(const Refl::ClassInfo*);
			Context& renderer(const Refl::ClassInfo*, const RendererFunc& func);

			inline Context* prev() const { return m_prev; }

			template<typename T>
			Context& renderer(const RendererFunc& func)
			{
				return renderer(T::static_refl_class_info(), func);
			}
		};

	private:
		struct NextPropertyName {
			String name;
			uint16_t usages = 1;
		};

		TreeMap<Refl::Struct*, PropertiesMap> m_properties;
		Vector<NextPropertyName> m_prop_names_stack;
		Vector<void*> m_context_stack;


		Context* m_ctx = nullptr;
		Object* m_object;
		Identifier m_destroy_id;
		size_t m_property_index = 0;

		PropertiesMap& build_props_map(Refl::Struct* self);

	public:
		UserData userdata;

		PropertyRenderer();
		~PropertyRenderer();

		Object* object() const;
		PropertyRenderer& object(Object* object, bool reset = true);

		bool render(RenderViewport* viewport) override;
		PropertyRenderer& render();
		Refl::Struct* struct_instance() const;
		const PropertiesMap& properties_map(Refl::Struct* self);

		PropertyRenderer& push_name(const String& name, uint16_t usages = 1);
		PropertyRenderer& pop_name();
		const String& property_name(const String& name);
		const String& property_name(Refl::Property* prop, const void* context);

		void* property_context(size_t stack_offset) const;
		bool render_property(void* object, Refl::Property* prop, bool read_only = false);
		bool render_struct_properties(void* object, class Refl::Struct* struct_class, bool read_only = false);

		virtual const char* name() const;
		static const char* static_name();

		inline size_t property_index() const { return m_property_index; }
		inline void* property_context() const { return m_context_stack.back(); }
		inline Context* renderer_context() const { return m_ctx; }
		inline PropertyRenderer& renderer_context(Context* ctx)
		{
			m_ctx = ctx;
			return *this;
		}

		static Context* static_global_renderer_context();
	};

}// namespace Engine
