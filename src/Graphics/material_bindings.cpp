#include <Graphics/material_bindings.hpp>

namespace Engine
{
	MaterialBindings::MaterialBindings(const std::initializer_list<Pair<Name, Binding>>& list) : m_bindings(list)
	{
		sort();
	}

	MaterialBindings::MaterialBindings(const Vector<Pair<Name, Binding>>& list) : m_bindings(list)
	{
		sort();
	}

	MaterialBindings::MaterialBindings(Vector<Pair<Name, Binding>>&& list) : m_bindings(std::move(list))
	{
		sort();
	}

	MaterialBindings& MaterialBindings::sort()
	{
		std::sort(m_bindings.begin(), m_bindings.end(),
		          [](const Element& a, const Element& b) -> bool { return a.first < b.first; });
		return *this;
	}

	MaterialBindings::Binding* MaterialBindings::find_or_create(const Name& name)
	{
		auto it = std::lower_bound(m_bindings.begin(), m_bindings.end(), name,
		                           [](const Element& elem, const Name& val) { return elem.first < val; });

		if (it != m_bindings.end() && name == it->first)
			return &it->second;

		it = m_bindings.emplace(it, name, MaterialBindings::Binding{});
		return &it->second;
	}

	const MaterialBindings::Binding* MaterialBindings::find(const Name& name) const
	{
		const MaterialBindings* current = this;

		do
		{
			auto it = std::lower_bound(m_bindings.begin(), m_bindings.end(), name,
			                           [](const Element& elem, const Name& val) { return elem.first < val; });

			if (it != m_bindings.end() && name == it->first)
			{
				return &it->second;
			}

			current = current->prev;
		} while (current);

		return nullptr;
	}

	bool MaterialBindings::unbind(const Name& name)
	{
		auto it = std::lower_bound(m_bindings.begin(), m_bindings.end(), name,
		                           [](const Element& elem, const Name& val) { return elem.first < val; });

		if (it != m_bindings.end() && name == it->first)
		{
			m_bindings.erase(it);
			return true;
		}

		return false;
	}

	void foo()
	{
		MaterialBindings bindings = {
		        {"hitproxy", int(0)},
		        {"color", Vector4f(0.5, 0.5, 1.0, 1.0)},
		};
	}
}// namespace Engine
