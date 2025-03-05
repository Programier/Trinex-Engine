#pragma once

#include <Core/etl/object_tree_node.hpp>
#include <Core/object.hpp>

namespace Engine
{

	class ENGINE_EXPORT Package : public ObjectTreeNode<Object, Object>
	{
		declare_class(Package, Object);

		bool can_add_object(Object* object) const;

	protected:
		Refl::Class* object_tree_child_class() const override;
		bool register_child(Object* object) override;

	public:
		delete_copy_constructors(Package);
		Package();

		bool add_object(Object* object);
		Package& remove_object(Object* object);

		const Vector<Object*>& objects() const;
		bool contains_object(const Object* object) const;
		bool contains_object(const StringView& name) const;
		bool save(BufferWriter* writer = nullptr, SerializationFlags flags = {}) override;
		friend class Object;
	};
}// namespace Engine
