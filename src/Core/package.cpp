#include <Core/archive.hpp>
#include <Core/compressor.hpp>
#include <Core/constants.hpp>
#include <Core/file_flag.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
	Package::Package()
	{
		flags(Object::IsPackage, true);
		flags(Object::StandAlone, true);
	}

	bool Package::can_add_object(Object* object, Name name) const
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

		if (contains_object(name))
		{
			error_log("Package", "Cannot add object to package. Object with name '%s' already exist in package!",
			          object->string_name().c_str());
			return false;
		}
		return true;
	}

	bool Package::add_new_object(Object* object, Name name)
	{
		if (can_add_object(object, name))
		{
			m_objects.insert_or_assign(name, object);
			return true;
		}
		return false;
	}

	bool Package::register_child(Object* object)
	{
		return add_new_object(object, object->name());
	}

	bool Package::unregister_child(Object* child)
	{
		if (child->owner() == this)
		{
			m_objects.erase(child->name());
		}

		return true;
	}

	bool Package::rename_child_object(Object* object, StringView new_name)
	{
		bool result = add_new_object(object, new_name);
		if (result)
		{
			m_objects.erase(object->name());
		}
		return result;
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

	Object* Package::find_object_private_no_recurse(const StringView& _name) const
	{
		Name object_name = _name;
		auto it          = m_objects.find(object_name);
		if (it == m_objects.end())
			return nullptr;
		return it->second;
	}

	Object* Package::find_child_object(StringView object_name, bool recursive) const
	{
		if (recursive)
		{
			StringView name = Strings::parse_name_identifier(object_name, &object_name);

			if (object_name.empty())
				return find_object_private_no_recurse(name);

			if (Object* object = find_object_private_no_recurse(name))
			{
				return object->find_child_object(object_name, true);
			}

			return nullptr;
		}


		return find_object_private_no_recurse(object_name);
	}

	const Package::ObjectMap& Package::objects() const
	{
		return m_objects;
	}

	bool Package::contains_object(const Object* object) const
	{
		return object ? object->package() == this : false;
	}

	bool Package::contains_object(const StringView& name) const
	{
		return find_child_object(name, false) != nullptr;
	}

	bool Package::save(BufferWriter* writer, Flags<SerializationFlags> serialization_flags)
	{
		if (!flags(Object::IsSerializable))
		{
			error_log("Package", "Cannot save non-serializable package!");
			return false;
		}

		bool result = true;

		for (auto& [name, object] : m_objects)
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

	Package::~Package()
	{
		auto objects = std::move(m_objects);
		for (auto& [name, object] : objects)
		{
			object->owner(nullptr);
		}
	}

	implement_engine_class(Package, Refl::Class::IsScriptable)
	{}
}// namespace Engine
