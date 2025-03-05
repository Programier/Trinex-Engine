#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>

namespace Engine
{
	Package::Package()
	{
		flags(Object::IsPackage, true);
		flags(Object::StandAlone, true);
	}

	bool Package::can_add_object(Object* object) const
	{
		if (!object)
			return false;

		if (object->is_noname())
		{
			error_log("Package", "Cannot add no name object to package!");
			return false;
		}

		if (!object->is_valid())
		{
			error_log("Package", "Cannot add invalid object to package");
			return false;
		}

		if (contains_object(object->name()))
		{
			error_log("Package", "Cannot add object to package. Object with name '%s' already exist in package!",
			          object->string_name().c_str());
			return false;
		}
		return true;
	}

	Refl::Class* Package::object_tree_child_class() const
	{
		return Object::static_class_instance();
	}

	bool Package::register_child(Object* object)
	{
		if (can_add_object(object))
			return ObjectTreeNode::register_child(object);
		return false;
	}

	bool Package::add_object(Object* object)
	{
		if (!object)
			return false;

		return object->owner(this);
	}

	Package& Package::remove_object(Object* object)
	{
		if (!object || object->package() != this)
			return *this;

		object->owner(nullptr);
		return *this;
	}

	const Vector<Object*>& Package::objects() const
	{
		return child_objects();
	}

	bool Package::contains_object(const Object* object) const
	{
		return object ? object->package() == this : false;
	}

	bool Package::contains_object(const StringView& name) const
	{
		return find_child_object(Strings::parse_name_identifier(name)) != nullptr;
	}

	bool Package::save(BufferWriter* writer, SerializationFlags serialization_flags)
	{
		if (!flags(Object::IsSerializable))
		{
			error_log("Package", "Cannot save non-serializable package!");
			return false;
		}

		bool result = true;

		for (Object* object : m_child_objects)
		{
			if (Package* sub_package = object->instance_cast<Package>())
			{
				result = sub_package->save(writer, serialization_flags);
				continue;
			}

			object->save(writer, serialization_flags);

			if (result == false)
			{
				return result;
			}
		}
		return result;
	}

	implement_engine_class(Package, Refl::Class::IsScriptable) {}
}// namespace Engine
