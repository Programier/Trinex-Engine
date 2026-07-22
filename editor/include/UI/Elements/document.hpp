#pragma once
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Document : public Element
	{
		trinex_ui_element(Document, Element);

	private:
		Refl::NativeType<void>* m_bindings;

	public:
		Document();
		~Document();

		inline Refl::NativeType<void>* bindings() const { return m_bindings; }
	};
}// namespace Trinex::UI
