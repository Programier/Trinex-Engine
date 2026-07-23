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

	Element& Element::bind(Name event, EventListener listener)
	{
		m_listeners[event] = etl::move(listener);
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

	bool Element::dispatch(Name name)
	{
		if (!name.is_valid())
			return false;

		Event event;
		event.sender = this;

		for (Element* element = this; element; element = element->owner())
		{
			auto it = element->m_listeners.find(name);

			if (it != element->m_listeners.end())
			{
				event.current = element;
				it->second(&event);

				if (event.handled() || !event.bubbling())
				{
					return event.handled();
				}
			}
		}

		return event.handled();
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

	Element& Element::update()
	{
		auto resolve_binding = [this](const Binding& binding) {
			return document()->bindings()->resolve(document(), binding.path.data(), binding.path.size());
		};

		for (auto& binding : m_bindings)
		{
			if (!(binding.path.mode & Markup::BindingPath::Mode::Read))
				continue;

			auto resolve = resolve_binding(binding);

			if (!binding.type->assign(binding.value, resolve.first, resolve.second))
			{
				trinex_error(Log::Editor, "Failed to update binding");
			}
		}

		auto flags = on_begin_update();

		if (flags & UpdateFlags::Readback)
		{
			for (auto& binding : m_bindings)
			{
				if (!(binding.path.mode & Markup::BindingPath::Mode::Write))
					continue;

				auto resolve = resolve_binding(binding);

				if (resolve.second == nullptr || !resolve.second->assign(resolve.first, binding.value, binding.type))
				{
					trinex_error(Log::Editor, "Failed to update binding");
				}
			}
		}

		if (flags & UpdateFlags::Childs)
		{
			for (Element* child : m_childs)
			{
				child->update();
			}
		}

		if (flags & UpdateFlags::End)
			on_end_update();

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

	Element::UpdateFlags Element::on_begin_update()
	{
		return UpdateFlags::Default;
	}

	Element& Element::on_end_update()
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
