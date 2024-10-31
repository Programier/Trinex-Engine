#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/reflection/scoped_type.hpp>
#include <Core/string_functions.hpp>

namespace Engine::Refl
{
	implement_reflect_type(ScopedType);

	static ScopedType* root = nullptr;

	Object* Object::static_root()
	{
		if (root == nullptr)
		{
			root = Object::new_instance<ScopedType>("Root");
		}

		return root;
	}

	static inline void static_initialize_childs(Object* root, bool force_recursive)
	{
		if (auto scope = Object::instance_cast<ScopedType>(root))
		{
			ScopedType::Locker lock(scope);

			for (auto& [name, child] : scope->childs())
			{
				if (child)
				{
					Object::static_initialize(child, force_recursive);
				}
			}
		}
	}

	void Object::static_initialize(Object* root, bool force_recursive)
	{
		if (root == nullptr)
			root = static_root();

		if (!root->m_is_initialized)
		{
			root->m_is_initialized = true;
			root->initialize();
			root->on_initialize(root);
			static_initialize_childs(root, force_recursive);
		}

		if (force_recursive)
		{
			static_initialize_childs(root, force_recursive);
		}
	}

	ScopedType& ScopedType::unregister_subobject(Object* subobject)
	{
		trinex_always_check(!is_locked(), "Object must be unlocked");
		m_childs.erase(subobject->name());
		return *this;
	}

	ScopedType& ScopedType::register_subobject(Object* subobject)
	{
		trinex_always_check(!is_locked(), "Object must be unlocked");
		m_childs.insert({subobject->name(), subobject});
		return *this;
	}

	Object* ScopedType::find(StringView name, FindFlags flags)
	{
		String instance = String(Strings::parse_name_identifier(name, &name));

		auto it        = m_childs.find(instance);
		Object* object = nullptr;

		if (it == m_childs.end() && !(flags & FindFlags::DisableReflectionCheck))
		{
			String controller_name = concat_scoped_name(full_name(), instance);
			ReflectionInitializeController().require(controller_name);
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

		if (object == nullptr && (flags & FindFlags::IsRequired) == FindFlags::IsRequired)
			throw EngineException(Strings::format("Failed to find reflection for '{}'", concat_scoped_name(full_name(), name)));

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
}// namespace Engine::Refl
