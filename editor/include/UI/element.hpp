#pragma once
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>
#include <Core/types/name.hpp>

namespace Trinex::UI
{
	class Type;

	class Element
	{
	public:
		using This = Element;

	private:
		Element* m_owner = nullptr;
		Vector<Element*> m_childs;
		u32 m_references = 1;

		static Type* initialize_type(Type* type);

	public:
		static Element* create(Name name);
		static Type* reflection();

		Element* attach(StringView type);
		Element& attach(Element* element);
		Element& deattach(Element* element);

		Element& render();

		u32 add_reference();
		u32 release();

		virtual bool on_begin_render();
		virtual Element& on_end_render();
		virtual Type* type() const;

		inline Element* owner() const { return m_owner; }
		inline const Vector<Element*>& childs() const { return m_childs; }
		inline u32 references() const { return m_references; }
		virtual ~Element();
	};

#define trinex_ui_element(name, super)                                                                                           \
public:                                                                                                                          \
	using This  = name;                                                                                                          \
	using Super = super;                                                                                                         \
	Trinex::UI::Type* type() const override;                                                                                     \
	static Trinex::UI::Type* reflection();                                                                                       \
                                                                                                                                 \
private:                                                                                                                         \
	static void initialize_reflection()

#define trinex_implement_ui_element(name)                                                                                        \
	Trinex::UI::Type* name::type() const                                                                                         \
	{                                                                                                                            \
		return name::reflection();                                                                                               \
	}                                                                                                                            \
	Trinex::UI::Type* name::reflection()                                                                                         \
	{                                                                                                                            \
		static UI::Type* s_type = Trinex::UI::ElementRegistry::instance()->bind<name>(#name);                                    \
		static auto registrar   = Trinex::LifeCycle::on_reflection_init([] {                                                     \
            s_type->parent<name::Super>();                                                                                     \
            name::initialize_reflection();                                                                                     \
        });                                                                                                                    \
		return s_type;                                                                                                           \
	}                                                                                                                            \
	trinex_on_pre_init()                                                                                                         \
	{                                                                                                                            \
		name::reflection();                                                                                                      \
	}                                                                                                                            \
	void name::initialize_reflection()

}// namespace Trinex::UI
