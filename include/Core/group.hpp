#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/array.hpp>
#include <Core/types/name.hpp>

namespace Trinex
{
	class trinex_api Group final
	{
		Array<Refl::Struct*> m_structs;

		Array<Group*> m_childs;
		Group* m_parent = nullptr;
		Name m_name;

		Group* find_subgroup(const char* name, usize len, bool create);

		Group();

	public:
		static Group* root();
		static Group* find(const String& name, bool create = false);
		static Group* find(const char* name, bool create = false);
		static Group* find(const char* name, usize len, bool create = false);

		Group* parent() const;
		const Array<Group*>& childs() const;
		const Name& name() const;
		const Array<class Refl::Struct*>& structs() const;
		Group& add_struct(class Refl::Struct*);
		Group& remove_struct(class Refl::Struct*);

		~Group();
	};
} // namespace Trinex
