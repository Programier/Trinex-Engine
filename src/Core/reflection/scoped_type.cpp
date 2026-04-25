#include <Core/reflection/scoped_type.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Trinex::Refl
{
	trinex_implement_reflect_type(Trinex::Refl::ScopedType) {}

	static ScopedType* root = nullptr;

	Object* Object::static_root()
	{
		if (root == nullptr)
		{
			root = Object::new_instance<ScopedType>("Root");
		}

		return root;
	}

	static inline void load_childs_reflection(Object* root, bool force_recursive)
	{
		if (auto scope = Object::instance_cast<ScopedType>(root))
		{
			ScopedType::Locker lock(scope);

			for (auto& [name, child] : scope->childs())
			{
				if (child)
				{
					Object::load_reflection(child, force_recursive);
				}
			}
		}
	}

	void Object::load_reflection(Object* root, bool force_recursive)
	{
		if (root == nullptr)
			root = static_root();

		if (!root->m_is_initialized)
		{
			root->m_is_initialized = true;
			root->initialize();
			root->on_initialize(root);
			load_childs_reflection(root, force_recursive);
		}

		if (force_recursive)
		{
			load_childs_reflection(root, force_recursive);
		}
	}

	ScopedType& ScopedType::unregister_subobject(Object* subobject)
	{
		trinex_verify_msg(!is_locked(), "Object must be unlocked");
		m_childs.erase(subobject->name());
		return *this;
	}

	ScopedType& ScopedType::register_subobject(Object* subobject)
	{
		trinex_verify_msg(!is_locked(), "Object must be unlocked");
		m_childs.insert({subobject->name(), subobject});
		return *this;
	}

	Object* ScopedType::find(StringView name, FindFlags flags)
	{
		String instance = String(Strings::parse_name_identifier(name, &name));

		auto it        = m_childs.find(instance);
		Object* object = nullptr;

		if (it == m_childs.end())
		{
			String handler = concat_scoped_name(full_name(), instance);
			//LifeCycle::execute(LifeCycle::ReflectionInit, handler.c_str());
			it = m_childs.find(instance);
		}

		if (it == m_childs.end())
		{
			if ((flags & FindFlags::CreateScope) == FindFlags::CreateScope)
			{
				object = Object::new_instance<ScopedType>(concat_scoped_name(full_name(), instance));
			}
			else
			{
				const bool is_required = (flags & FindFlags::IsRequired) == FindFlags::IsRequired;
				trinex_assert_fmt(!is_required, "Failed to find reflection for '%s'",
				                  concat_scoped_name(full_name(), name).c_str());
				return nullptr;
			}
		}
		else
		{
			object = it->second;
		}

		if (object && !name.empty())
		{
			object = object->find(name, flags);
		}

		const bool is_found = object != nullptr || (flags & FindFlags::IsRequired) != FindFlags::IsRequired;
		trinex_assert_fmt(is_found, "Failed to find reflection for '%s'", concat_scoped_name(full_name(), name).c_str());

		return object;
	}

	const Map<String, Object*>& ScopedType::childs() const
	{
		return m_childs;
	}

	void ScopedType::lock() const
	{
		++m_lock_count;
	}

	void ScopedType::unlock() const
	{
		if (m_lock_count > 0)
		{
			--m_lock_count;
		}
	}

	ScopedType::~ScopedType()
	{
		auto childs = std::move(m_childs);

		for (auto& [name, child] : childs)
		{
			Object::destroy_instance(child);
		}

		if (this == root)
			root = nullptr;
	}
}// namespace Trinex::Refl
