#pragma once
#include <Core/etl/flat_map.hpp>
#include <Core/types/path.hpp>
#include <UI/element.hpp>

namespace Trinex::UI
{
	class Document : public Element
	{
		trinex_ui_element(Document, Element);

	private:
		Refl::NativeType<void>* m_bindings;
		FlatMap<Name, Element*> m_elements;
		StyleSheet m_style_sheet;
		Vector<Path> m_dependencies;
		bool m_open = false;

	public:
		Document();
		~Document();

		bool load(StringView source, const Path& path);
		bool load(const Path& path);

		Document& open();
		Document& close();
		Document& register_element(Name id, Element* element);
		bool is_open() const;
		bool is_closed() const;
		Element* find_element(Name id) const;

		inline Refl::NativeType<void>* bindings() const { return m_bindings; }
		inline StyleSheet* style_sheet() { return &m_style_sheet; }
		inline const StyleSheet* style_sheet() const { return &m_style_sheet; }
		inline const Vector<Path>& dependencies() const { return m_dependencies; }
	};
}// namespace Trinex::UI
