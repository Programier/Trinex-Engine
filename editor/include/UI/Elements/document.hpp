#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Document : public Element
	{
		trinex_ui_element(Document, Element);

	private:
		Refl::NativeType<void>* m_bindings;
		bool m_open = false;

	public:
		Document();
		~Document();

		Document& open();
		Document& close();
		bool is_open() const;
		bool is_closed() const;

		inline Refl::NativeType<void>* bindings() const { return m_bindings; }
	};
}// namespace Trinex::UI
