#pragma once
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>
#include <Core/types/name.hpp>
#include <UI/types.hpp>

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
		using This = Element;
		struct Binding {
			void* value;
			const Refl::Type* type;
			Markup::BindingPath path;
		};

	private:
		Element* m_owner     = nullptr;
		Document* m_document = nullptr;
		Vector<Element*> m_childs;
		Vector<Binding> m_bindings;
		u32 m_references = 1;

		static Refl::Type* initialize_type(Refl::Type* type);

	public:
		static Element* create(Name name);
		static Refl::Type* reflection();
		static Element* cast(void* src, const Refl::Type* type);

		Element& bind(void* value, const Refl::Type* type, const Markup::BindingPath& path);
		Element& unbind(void* value);
		usize binding_index(void* value);

		Element* attach(StringView type);
		Element& attach(Element* element);
		Element& deattach(Element* element);
		Element& document(Document* document);

		Element& render();

		u32 add_reference();
		u32 release();

		virtual Element& on_update();
		virtual bool on_begin_render();
		virtual Element& on_end_render();
		virtual Refl::Type* type() const;

		inline Element* owner() const { return m_owner; }
		inline Document* document() const { return m_document; }
		inline const Vector<Element*>& childs() const { return m_childs; }
		inline u32 references() const { return m_references; }
		inline const Vector<Binding>& bindings() const { return m_bindings; }
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

}// namespace Trinex::UI
