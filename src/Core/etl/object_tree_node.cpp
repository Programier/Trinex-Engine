#include <Core/etl/object_tree_node.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <algorithm>

namespace Engine
{
	static bool predicate(const Object* object, Identifier id)
	{
		return object->name().index() < id;
	}

	bool ObjectTreeNodeStatics::lower_bound(iterator begin, iterator end, iterator& out, Object* object, Refl::Class* check_class)
	{
		if (!object->class_instance()->is_a(check_class))
		{
			return false;
		}

		Identifier id = object->name().index();
		out           = std::lower_bound(begin, end, id, predicate);
		return true;
	}

	Object* ObjectTreeNodeStatics::find(const_iterator begin, const_iterator end, StringView full_name)
	{
		Name name = Strings::parse_name_identifier(full_name, &full_name);

		if (auto child = find(begin, end, name))
		{
			if (!full_name.empty())
			{
				return child->find_child_object(full_name);
			}
			return child;
		}
		return nullptr;
	}

	Object* ObjectTreeNodeStatics::find(const_iterator begin, const_iterator end, Name name)
	{
		Identifier id = name.index();
		auto place    = std::lower_bound(begin, end, id, predicate);

		if (place != end)
		{
			if ((*place)->name() == name)
				return *place;
		}
		return nullptr;
	}
}// namespace Engine
