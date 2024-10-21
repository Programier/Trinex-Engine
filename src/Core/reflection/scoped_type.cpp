#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/reflection/scoped_type.hpp>
#include <Core/string_functions.hpp>

namespace Engine::Refl
{
	implement_reflect_type(ScopedType);

	Object* Object::static_root()
	{
		static ScopedType* root = nullptr;

		if (root == nullptr)
		{
			root = Object::new_instance<ScopedType>("Root");
			root->on_destroy += [](Object*) { root = nullptr; };
		}

		return root;
	}

	ScopedType& ScopedType::unregister_subobject(Object* subobject)
	{
		m_childs.erase(subobject->name());
		return *this;
	}

	ScopedType& ScopedType::register_subobject(Object* subobject)
	{
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

	ScopedType::~ScopedType()
	{
		auto childs = std::move(m_childs);

		for (auto& [name, child] : childs)
		{
			Object::destroy_instance(child);
		}
	}
}// namespace Engine::Refl
