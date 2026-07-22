#include <Core/etl/algorithm.hpp>
#include <UI/Elements/document.hpp>
#include <UI/element.hpp>
#include <UI/reflection.hpp>

namespace Trinex::UI
{
	Refl::Type* Element::initialize_type(Refl::Type* type)
	{
		return type;
	}

	Refl::Type* Element::reflection()
	{
		static Refl::Type* s_type = initialize_type(Refl::NativeType<Element>::instance());
		return s_type;
	}

	Element* Element::cast(void* src, const Refl::Type* type)
	{
		auto target = Element::reflection();

		while (type)
		{
			if (type == target)
				return static_cast<Element*>(src);

			type = type->parent();
		}

		return nullptr;
	}

	Element* Element::create(Name name)
	{
		if (auto type = Refl::ElementRegistry::instance()->find(name))
		{
			return static_cast<Element*>(type->factory());
		}

		return nullptr;
	}

	Element& Element::bind(void* value, const Refl::Type* type, const Markup::BindingPath& path)
	{
		const usize size = m_bindings.size();
		const usize idx  = binding_index(value);

		if (idx == size)
		{
			m_bindings.push_back({.value = value, .type = type, .path = path});
		}
		return *this;
	}

	Element& Element::unbind(void* value)
	{
		auto predicate = [value](const Binding& binding) { return binding.value == value; };
		auto it        = etl::find_if(m_bindings.begin(), m_bindings.end(), predicate);
		if (it != m_bindings.end())
			m_bindings.erase_unordered(it);
		return *this;
	}

	usize Element::binding_index(void* value)
	{
		auto predicate = [value](const Binding& binding) { return binding.value == value; };
		return etl::find_if(m_bindings.begin(), m_bindings.end(), predicate) - m_bindings.begin();
	}

	Element* Element::attach(StringView type)
	{
		if (auto element = create(type))
		{
			element->m_owner = this;
			element->document(m_document);
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
				element->document(m_document);
				return *this;
			}

			element->add_reference();

			if (element->m_owner)
			{
				element->m_owner->deattach(element);
			}

			element->m_owner = this;
			element->document(m_document);
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
		element->document(nullptr);
		element->release();
		return *this;
	}

	Element& Element::document(Document* document)
	{
		m_document = document;

		for (Element* child : m_childs)
		{
			child->document(document);
		}

		return *this;
	}

	Element& Element::render()
	{
		for (auto& binding : m_bindings)
		{
			auto resolve = document()->bindings()->resolve(this, binding.path.data(), binding.path.size());

			if (!binding.type->assign(binding.value, resolve.first, resolve.second))
			{
				trinex_error(Log::Editor, "Failed to update binding");
			}
		}

		on_update();

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

	Element& Element::on_update()
	{
		return *this;
	}

	bool Element::on_begin_render()
	{
		return true;
	}

	Element& Element::on_end_render()
	{
		return *this;
	}

	Refl::Type* Element::type() const
	{
		return Element::reflection();
	}

	Element::~Element()
	{
		for (Element* child : m_childs)
		{
			child->m_owner = nullptr;
			child->document(nullptr);
			child->release();
		}

		m_childs.clear();
	}
}// namespace Trinex::UI
