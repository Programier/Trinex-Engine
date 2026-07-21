#include <Core/etl/algorithm.hpp>
#include <UI/element.hpp>
#include <UI/element_registry.hpp>

namespace Trinex::UI
{
	Type* Element::initialize_type(Type* type)
	{
		return type;
	}

	Type* Element::reflection()
	{
		static Type* s_type = initialize_type(Type::instance<Element>());
		return s_type;
	}

	Element* Element::create(Name name)
	{
		if (auto type = ElementRegistry::instance()->find(name))
		{
			return static_cast<Element*>(type->create());
		}

		return nullptr;
	}

	Element* Element::attach(StringView type)
	{
		if (auto element = create(type))
		{
			element->m_owner = this;
			m_childs.push_back(element);
			return element;
		}

		return nullptr;
	}

	Element& Element::attach(Element* element)
	{
		if (element)
		{
			if (element->m_owner == this)
			{
				return *this;
			}

			element->add_reference();

			if (element->m_owner)
			{
				element->m_owner->deattach(element);
			}

			element->m_owner = this;
			m_childs.push_back(element);
		}

		return *this;
	}

	Element& Element::deattach(Element* element)
	{
		if (element == nullptr || element->m_owner != this)
			return *this;

		auto it = etl::find(m_childs.begin(), m_childs.end(), element);

		if (it == m_childs.end())
		{
			return *this;
		}

		m_childs.erase(it);
		element->m_owner = nullptr;
		element->release();
		return *this;
	}

	Element& Element::render()
	{
		if (on_begin_render())
		{
			for (Element* child : m_childs)
			{
				child->render();
			}

			on_end_render();
		}
		return *this;
	}

	u32 Element::add_reference()
	{
		return ++m_references;
	}

	u32 Element::release()
	{
		trinex_assert(m_references > 0);
		const u32 count = --m_references;

		if (count == 0)
		{
			trx_delete this;
		}

		return count;
	}

	bool Element::on_begin_render()
	{
		return true;
	}

	Element& Element::on_end_render()
	{
		return *this;
	}

	Type* Element::type() const
	{
		return Element::reflection();
	}

	Element::~Element()
	{
		for (Element* child : m_childs)
		{
			child->m_owner = nullptr;
			child->release();
		}

		m_childs.clear();
	}
}// namespace Trinex::UI
