#include <Core/constants.hpp>
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
		return Object::static_reflection();
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

	static FORCE_INLINE Package* find_next_package(Package* package, const StringView& name, bool create)
	{
		Package* next_package = package->find_child_object_checked<Package>(name);
		if (next_package == nullptr && create && !name.empty())
		{
			next_package = Object::new_instance<Package>(name, package);
		}
		return next_package;
	}

	Package* Package::find_package(StringView name, bool create)
	{
		if (name.empty())
			return nullptr;

		Package* package        = this;
		StringView package_name = Strings::parse_name_identifier(name, &name);

		while (!package_name.empty())
		{
			package      = find_next_package(package, package_name, create);
			package_name = Strings::parse_name_identifier(name, &name);
		}

		return package;
	}


	trinex_implement_engine_class(Package, Refl::Class::IsScriptable) {}
}// namespace Engine
