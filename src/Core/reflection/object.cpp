#include <Core/reflection/class.hpp>
#include <Core/reflection/object.hpp>

namespace Trinex::Refl
{
	Object::Object(Name name, u64 flags) : m_owner(nullptr), m_flags(flags), m_name(name) {}

	Class* Object::reflection() const
	{
		return class_of<Object>();
	}

	Object* Object::resolve(const Class* base)
	{
		return is(base) ? this : nullptr;
	}

	const Object* Object::resolve(const Class* base) const
	{
		return is(base) ? this : nullptr;
	}

	Object* Object::find(Name name, const Class* base) const
	{
		return nullptr;
	}

	Object* Object::lookup(StringView path, const Class* base) const
	{
		const Object* scope = this;
		Object* object      = nullptr;

		if (path.starts_with("::"))
		{
			scope = Object::root();
			path.remove_prefix(2);

			if (path.empty())
				return base == nullptr || scope->is(base) ? const_cast<Object*>(scope) : nullptr;
		}

		while (!path.empty())
		{
			const StringView entry = Strings::split(path, "::");

			if (entry.empty())
				return nullptr;

			object = scope->find(Name(entry));

			if (object == nullptr)
				return nullptr;

			scope = object;
		}

		return base == nullptr || object->is(base) ? object : nullptr;
	}

	usize IndirectResolver<Object>::parse(Object*& result, Refl::Class* base, StringView value)
	{
		const usize end       = value.find_first_of(",} \t\r\n");
		const StringView path = value.substr(0, end);

		if (path == "null")
		{
			result = nullptr;
			return path.size();
		}

		if (Object* object = Object::root()->lookup(path, base))
		{
			result = object;
			return path.size();
		}

		return 0;
	}

	bool IndirectResolver<Object>::format(const Object* object, String& value)
	{
		if (object == nullptr)
		{
			value += "null";
			return true;
		}

		String path;
		const Object* root = Object::root();

		while (object != root)
		{
			if (object == nullptr || !object->name().is_valid())
				return false;

			path              = object->name().to_string() + (path.empty() ? "" : "::" + path);
			Ref<Object> owner = object->owner();
			object            = owner.value();
		}

		value += "::" + path;
		return true;
	}

	bool Object::owner(Object* object)
	{
		if (m_owner == object)
			return true;

		if (object == this)
			return false;

		Ref<Object> keep_alive = retain_ref(this);

		if (auto lock = m_owner.lock())
		{
			if (!lock->unregister_child(this))
			{
				return false;
			}
		}

		if (object == nullptr)
			return true;

		if (!object->register_child(this))
			return false;

		m_owner = object;
		return true;
	}

	bool Object::is(const Class* base) const
	{
		if (Class* instance = reflection())
			return base && instance->is(base);

		return false;
	}

	bool Object::register_child(Object* object)
	{
		return false;
	}

	bool Object::unregister_child(Object* object)
	{
		return false;
	}
} // namespace Trinex::Refl
