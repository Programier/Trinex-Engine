#include <archive.hpp>
#include <cassert>
#include <model.hpp>

namespace Reflector
{
	trinex_implement_reflector_type(Enum, values);
	trinex_implement_reflector_type(Scope, objects);
	trinex_implement_reflector_type(Namespace);
	trinex_implement_reflector_type(Struct);
	trinex_implement_reflector_type(Class);
	trinex_implement_reflector_type(Module);
	trinex_implement_reflector_type(Function, type, args, flags);
	trinex_implement_reflector_type(Property, type, value, flags);

	void serialize(Archive& ar, Object*& object)
	{
		ObjectKind kind = object ? object->kind() : ObjectKind::Object;
		ar(kind);

		if (ar.reader())
		{
			assert(object == nullptr);
			object = Object::create(kind);
		}

		object->serialize(ar);
	}

	Object* Object::create(ObjectKind kind)
	{
		switch (kind)
		{
			case ObjectKind::Object: return new Object();
			case ObjectKind::Enum: return new Enum();
			case ObjectKind::Scope: return new Scope();
			case ObjectKind::Namespace: return new Namespace();
			case ObjectKind::Struct: return new Struct();
			case ObjectKind::Class: return new Class();
			case ObjectKind::Module: return new Module();
			case ObjectKind::Function: return new Function();
			case ObjectKind::Property: return new Property();
			default: return nullptr;
		}
	}

	Object* Object::find(std::string_view, ObjectKind)
	{
		return nullptr;
	}

	ObjectKind Object::kind() const
	{
		return ObjectKind::Object;
	}

	bool Object::is_a(ObjectKind kind) const
	{
		return kind == ObjectKind::Object;
	}

	void Object::serialize(Archive& ar)
	{
		ar(name, access, metadata);
	}

	Object::~Object() {}

	Object* Scope::find(std::string_view name, ObjectKind kind)
	{
		if (name.starts_with("::"))
		{
			Object* root = this;
			while (root->owner) root = root->owner;
			return root->find(name.substr(2), kind);
		}
		const auto separator  = name.find("::");
		const auto local_name = name.substr(0, separator);

		for (const auto& object : objects)
		{
			if (!object || (object->kind() != ObjectKind::Module && object->name != local_name))
				continue;

			if (separator != std::string_view::npos)
			{
				if (auto* found = object->find(name.substr(separator + 2), kind))
					return found;
			}
			else if (object->kind() == kind)
			{
				return object;
			}
		}

		return nullptr;
	}

	Scope::~Scope()
	{
		for (Object* object : objects)
		{
			delete object;
		}
	}
}// namespace Reflector
