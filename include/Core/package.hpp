#pragma once

#include <Core/callback.hpp>
#include <Core/etl/map.hpp>
#include <Core/object.hpp>

namespace Engine
{

	class ENGINE_EXPORT Package : public Object
	{
		declare_class(Package, Object);

	public:
		using ObjectMap = Map<Name, Object*, Name::HashFunction>;


	private:
		ObjectMap m_objects;

		Object* find_object_private_no_recurse(const StringView& name) const;
		bool can_add_object(Object* object, Name name) const;

	protected:
		bool add_new_object(Object* object, Name name);
		bool register_child(Object* child) override;
		bool unregister_child(Object* child) override;
		bool rename_child_object(Object* object, StringView new_name) override;

	public:
		delete_copy_constructors(Package);
		Package();

		bool add_object(Object* object);
		Package& remove_object(Object* object);
		Object* find_child_object(StringView name, bool recursive = true) const override;

		const ObjectMap& objects() const;
		bool contains_object(const Object* object) const;
		bool contains_object(const StringView& name) const;
		bool save(BufferWriter* writer = nullptr, Flags<SerializationFlags> flags = {}) override;

		~Package();
		friend class Object;
	};
}// namespace Engine
