#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/vector.hpp>
#include <Core/name.hpp>

namespace Engine
{
	class ENGINE_EXPORT Group final
	{
		Vector<Refl::Struct*> m_structs;

		Vector<Group*> m_childs;
		Group* m_parent = nullptr;
		Name m_name;

		Group* find_subgroup(const char* name, size_t len, bool create);

		Group();

	public:
		static Group* root();
		static Group* find(const String& name, bool create = false);
		static Group* find(const char* name, bool create = false);
		static Group* find(const char* name, size_t len, bool create = false);

		Group* parent() const;
		const Vector<Group*>& childs() const;
		const Name& name() const;
		const Vector<class Refl::Struct*>& structs() const;
		Group& add_struct(class Refl::Struct*);
		Group& remove_struct(class Refl::Struct*);

		~Group();
	};
}// namespace Engine
