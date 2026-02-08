#pragma once
#include <Core/userdata.hpp>
#include <Graphics/imgui.hpp>

namespace Engine
{
	namespace Refl
	{
		struct ClassInfo;
		struct PropertyChangedEvent;
	}// namespace Refl

	class ScriptFunction;

	namespace UI
	{
		struct PropertyRenderingFlags {
			enum Enum : EnumerateType
			{
				Undefined   = 0,
				RenderNames = 1 << 0,
			};

			trinex_bitfield_enum_struct(PropertyRenderingFlags, EnumerateType);
		};

		class PropertyRenderer
		{
		public:
			using PropertiesMap = TreeMap<String, Vector<Refl::Property*>>;
			using RendererFunc  = Function<bool(PropertyRenderer* renderer, Refl::Property* prop, bool read_only)>;

			class Context
			{
			public:
				using BeginRenderingFunc = Function<bool(PropertyRenderer* renderer)>;
				using EndRenderingFunc   = Function<void(PropertyRenderer* renderer, bool)>;

			private:
				Map<const Refl::ClassInfo*, RendererFunc> m_renderers;
				Context* m_prev = nullptr;

			public:
				virtual PropertyRenderingFlags flags() const;

				virtual bool on_begin_rendering(PropertyRenderer* renderer);
				virtual Context& on_end_rendering(PropertyRenderer* renderer, bool rendered);
				virtual bool on_begin_group(PropertyRenderer* renderer, const String& group);
				virtual Context& on_end_group(PropertyRenderer* renderer, const String& group, bool open);

				virtual uint_t columns() const;
				virtual Context& column(uint_t index);
				virtual Context& next_row(ImGuiTableRowFlags row_flags = 0, float row_min_height = 0.f);
				virtual float cell_width() const;

			public:
				Context(Context* prev = nullptr) : m_prev(prev) {}

				const RendererFunc& renderer(const Refl::ClassInfo*) const;
				const RendererFunc* renderer_ptr(const Refl::ClassInfo*) const;
				Context& renderer(const Refl::ClassInfo*, const RendererFunc& func);

				inline Context* prev() const { return m_prev; }

				template<typename T>
				Context& renderer(const RendererFunc& func)
				{
					return renderer(T::static_refl_class_info(), func);
				}
			};

		private:
			struct ContextEntry {
				Context* context;
				PropertyRenderingFlags flags;
				bool active;
			};

			Vector<StringView> m_names_stack;
			Vector<ContextEntry> m_context_stack;
			Refl::PropertyChangedEvent* m_event = nullptr;

		public:
			UserData userdata;

			PropertyRenderer& propagate_property_event();
			PropertyRenderer& propagate_property_event(void* ctx, Refl::Property* property);
			inline StringView property_name() const { return m_names_stack.back(); }
			StringView property_name(StringView name);
			StringView property_name(Refl::Property* prop, const void* context);

			bool begin(Context* ctx = nullptr);
			PropertyRenderer& end();

			void* property_address() const;
			Refl::Property* property() const;
			bool render_property(void* object, Refl::Property* prop, bool read_only = false, StringView name = "");
			bool render_properties(void* object, class Refl::Struct* struct_class, bool read_only = false, StringView name = "");

			inline PropertyRenderingFlags property_rendering_flags() const { return m_context_stack.back().flags; }
			inline Context* context() const { return m_context_stack.back().context; }

			static PropertyRenderer* static_renderer();
			static Context* static_context();
		};
	}// namespace UI
}// namespace Engine
