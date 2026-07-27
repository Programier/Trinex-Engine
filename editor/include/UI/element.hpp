#pragma once
#include <Core/etl/flat_map.hpp>
#include <Core/etl/function.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>
#include <Core/types/name.hpp>
#include <UI/style_sheet.hpp>
#include <UI/types.hpp>
#include <imgui.h>

namespace Trinex::UI
{
	class Document;

	namespace Refl
	{
		class Type;

		template<typename T>
		class NativeType;
	}// namespace Refl

	class Element
	{
	public:
		using EventListener = Function<void(Event* event)>;

		struct UpdateFlags {
			enum Enum : u16
			{
				Undefined  = 0,
				Readback   = 1 << 0,
				Childs     = 1 << 1,
				End        = 1 << 2,
				Hovered    = 1 << 3,
				Active     = 1 << 4,
				Focused    = 1 << 5,
				Disabled   = 1 << 6,
				Checked    = 1 << 7,
				Selected   = 1 << 8,
				Open       = 1 << 9,
				Closed     = 1 << 10,
				Empty      = 1 << 11,
				FirstChild = 1 << 12,
				LastChild  = 1 << 13,
				Odd        = 1 << 14,
				Even       = 1 << 15,

				Default = Childs | End,
			};

			trinex_bitfield_enum_struct(UpdateFlags, u16);
		};

		using This = Element;
		struct Binding {
			void* value;
			const Refl::Type* type;
			Markup::BindingPath path;
		};

		class CurrentScope
		{
		private:
			Element* m_previous;

		public:
			CurrentScope(Element* element);
			~CurrentScope();
		};

	private:
		Element* m_owner     = nullptr;
		Document* m_document = nullptr;
		Name m_id;
		Vector<Element*> m_childs;
		Vector<Binding> m_bindings;
		Vector<Name> m_styles;
		StyleInstance m_style_instance;
		StyleState m_style_state = StyleState::Undefined;
		FlatMap<Name, EventListener> m_listeners;
		u32 m_references = 1;

		static Refl::Type* initialize_type(Refl::NativeType<Element>* type);

	protected:
		static void push_style_var(ImGuiStyleVar var, f32 value);
		static void push_style_var(ImGuiStyleVar var, Unit value);
		static void push_style_var(ImGuiStyleVar var, Size value);
		static void push_style_var(ImGuiStyleVar var, const Vec2& value);
		static void push_style_color(ImGuiCol color, const Vec4& value);
		static void push_style_var(ImGuiStyleVar var, const ImVec2& value);
		static void push_style_color(ImGuiCol color, const ImVec4& value);

	public:
		Vec2 spacing;
		Vec2 inner_spacing;
		f32 alpha = 1.f;
		f32 indent;
		Vec4 text_color          = {0.80f, 0.80f, 0.80f, 1.00f};
		Vec4 border_color        = {0.12f, 0.12f, 0.18f, 0.50f};
		Vec4 border_shadow_color = {0.00f, 0.00f, 0.00f, 0.00f};

	public:
		static Element* create(Name name);
		static Refl::Type* reflection();
		static Element* cast(void* src, const Refl::Type* type);
		static Element* current();
		static f32 resolve(Unit unit, Axis axis);
		static ImVec2 resolve(Size size);

		Element& bind(void* value, const Refl::Type* type, const Markup::BindingPath& path);
		Element& bind(Name event, EventListener listener);
		Element& unbind(void* value);
		usize binding_index(void* value);
		bool dispatch(Name name);

		Element* attach(StringView type);
		Element& attach(Element* element);
		Element& deattach(Element* element);
		Element& clear();
		Element& document(Document* document);
		Element& style(Name name);
		Element& inline_property(const StyleProperty& property);
		Element& apply_styles();
		Element& update_style_state(UpdateFlags flags);

		Element& update();

		u32 add_reference();
		u32 release();

		virtual Element& push_style();
		virtual Element& pop_style();

		virtual UpdateFlags on_begin_update();
		virtual Element& on_end_update(UpdateFlags flags);
		virtual Refl::Type* type() const;

		static inline UpdateFlags readback_if(bool condition)
		{
			return condition ? UpdateFlags::Readback : UpdateFlags::Undefined;
		}

		static UpdateFlags item_state_flags(UpdateFlags flags = UpdateFlags::Undefined);

		template<typename T>
		Element& bind(T* value, const Markup::BindingPath& path)
		{
			return Element::bind(value, Refl::NativeType<T>::instance(), path);
		}

		inline Element* owner() const { return m_owner; }
		inline Document* document() const { return m_document; }
		inline const Name& id() const { return m_id; }
		inline const Vector<Element*>& childs() const { return m_childs; }
		inline const Vector<Name>& styles() const { return m_styles; }
		inline StyleState style_state() const { return m_style_state; }
		inline u32 references() const { return m_references; }
		inline const Vector<Binding>& bindings() const { return m_bindings; }

		template<typename T>
		inline T* as()
		{
			return static_cast<T*>(this);
		}

		template<typename T>
		inline const T* as() const
		{
			return static_cast<const T*>(this);
		}
		virtual ~Element();
	};

#define trinex_ui_element(name, super)                                                                                           \
public:                                                                                                                          \
	using This  = name;                                                                                                          \
	using Super = super;                                                                                                         \
	Trinex::UI::Refl::Type* type() const override;                                                                               \
	static Trinex::UI::Refl::NativeType<name>* reflection();                                                                     \
                                                                                                                                 \
private:                                                                                                                         \
	static void initialize_reflection()

#define trinex_implement_ui_element(name)                                                                                        \
	Trinex::UI::Refl::Type* name::type() const                                                                                   \
	{                                                                                                                            \
		return name::reflection();                                                                                               \
	}                                                                                                                            \
	Trinex::UI::Refl::NativeType<name>* name::reflection()                                                                       \
	{                                                                                                                            \
		static Trinex::UI::Refl::NativeType<name>* s_type = Trinex::UI::Refl::ElementRegistry::instance()->bind<name>(#name);    \
		static auto registrar = Trinex::LifeCycle::on_reflection_init([] { name::initialize_reflection(); });                    \
		return s_type;                                                                                                           \
	}                                                                                                                            \
	trinex_on_pre_init()                                                                                                         \
	{                                                                                                                            \
		name::reflection();                                                                                                      \
	}                                                                                                                            \
	void name::initialize_reflection()

#define trinex_ui_bind_property(name, type) reflection()->bind(#name, &This::name, Trinex::UI::Refl::Property::type)
#define trinex_ui_bind_virtual_property(name, type)                                                                              \
	reflection()->bind(#name, &This::name, &This::name, Trinex::UI::Refl::Property::type)

}// namespace Trinex::UI
