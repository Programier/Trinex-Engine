#include <Graphics/material_bindings.hpp>

namespace Engine
{
	MaterialBindings::MaterialBindings(const std::initializer_list<Container::value_type>& list) : m_bindings(list) {}
	MaterialBindings::MaterialBindings(const Container& list) : m_bindings(list) {}
	MaterialBindings::MaterialBindings(Container&& list) : m_bindings(std::move(list)) {}

	MaterialBindings::Binding* MaterialBindings::find_or_create(const Name& name)
	{
		return &m_bindings[name];
	}

	const MaterialBindings::Binding* MaterialBindings::find(const Name& name) const
	{
		const MaterialBindings* current = this;

		do
		{
			auto it = m_bindings.find(name);

			if (it != m_bindings.end())
			{
				return &it->second;
			}

			current = current->prev;
		} while (current);

		return nullptr;
	}

	bool MaterialBindings::unbind(const Name& name)
	{
		auto it = m_bindings.find(name);

		if (it != m_bindings.end())
		{
			m_bindings.erase(it);
			return true;
		}

		return false;
	}
}// namespace Engine
