#include <Core/etl/algorithm.hpp>
#include <UI/Elements/document.hpp>
#include <UI/element.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace Trinex::UI
{
	static thread_local Element* s_current = nullptr;

	Refl::Type* Element::initialize_type(Refl::NativeType<Element>* type)
	{
		type->bind("id", &Element::m_id);
		type->bind("alpha", &Element::alpha, Refl::Property::Style);
		type->bind("spacing", &Element::spacing, Refl::Property::Style);
		type->bind("inner_spacing", &Element::inner_spacing, Refl::Property::Style);
		type->bind("indent", &Element::indent, Refl::Property::Style);
		type->bind("text_color", &Element::text_color, Refl::Property::Style);
		type->bind("border_color", &Element::border_color, Refl::Property::Style);
		type->bind("border_shadow_color", &Element::border_shadow_color, Refl::Property::Style);
		return type;
	}

	Refl::Type* Element::reflection()
	{
		static Refl::Type* s_type = initialize_type(Refl::ElementRegistry::instance()->bind<Element>("Element"));
		return s_type;
	}

	Element::UpdateFlags Element::item_state_flags(UpdateFlags flags)
	{
		if (ImGui::IsItemHovered())
		{
			flags |= UpdateFlags::Hovered;
		}

		if (ImGui::IsItemActive())
		{
			flags |= UpdateFlags::Active;
		}

		if (ImGui::IsItemFocused())
		{
			flags |= UpdateFlags::Focused;
		}

		return flags;
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

	Element* Element::current()
	{
		return s_current;
	}

	Element::CurrentScope::CurrentScope(Element* element) : m_previous(s_current)
	{
		s_current = element;
	}

	Element::CurrentScope::~CurrentScope()
	{
		s_current = m_previous;
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

	Element& Element::clear()
	{
		for (Element* child : m_childs)
		{
			child->m_owner = nullptr;
			child->document(nullptr);
			child->release();
		}

		m_childs.clear();
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

	Element& Element::style(Name name)
	{
		if (name.is_valid())
		{
			m_styles.push_back(name);
			m_style_instance.clear();
		}

		return *this;
	}

	static bool assign_style_property(Element* element, const StyleProperty& property)
	{
		if (property.path.size() == 1 && property.path.front() == "transition")
		{
			return true;
		}

		auto type = element->type();

		auto visitor = [&]<typename T>(const T& value) -> bool {
			Element::CurrentScope current(element);

			Refl::PropertyRef dst = {
			        .address = element,
			        .type    = type,
			        .field   = property.path.data(),
			        .fields  = property.path.size(),
			};

			Refl::ConstValueRef src = {
			        .address = &value,
			        .type    = UI::Refl::NativeType<T>::instance(),
			};

			return Refl::Type::assign(dst, src, Refl::Property::Style);
		};

		return etl::visit(visitor, property.value.value);
	}

	Element& Element::apply_styles()
	{
		if (document() && document()->style_sheet())
		{
			auto source = [this](const StyleRuleVisitor& visitor) {
				document()->style_sheet()->resolve(this, m_style_state, visitor);
			};

			auto apply = [this](const StyleProperty& property) {
				if (!assign_style_property(this, property))
				{
					trinex_error(Log::Editor, "Failed to assign style property '%s' of element '%s' at %u:%u",
					             property.path.empty() ? "" : property.path.front().c_str(), type()->name().c_str(),
					             property.location.line, property.location.column);
				}
			};

			m_style_instance.update(document()->style_sheet()->revision(), m_style_state, source, apply);
		}

		return *this;
	}

	Element& Element::update_style_state(UpdateFlags flags)
	{
		m_style_state = StyleState::Undefined;

		if (flags & UpdateFlags::Hovered)
		{
			m_style_state |= StyleState::Hover;
		}

		if (flags & UpdateFlags::Active)
		{
			m_style_state |= StyleState::Active;
		}

		if (flags & UpdateFlags::Focused)
		{
			m_style_state |= StyleState::Focus;
		}

		if (flags & UpdateFlags::Disabled)
		{
			m_style_state |= StyleState::Disabled;
		}

		return *this;
	}

	Element& Element::update()
	{
		CurrentScope current(this);

		apply_styles();

		for (auto& binding : m_bindings)
		{
			if (!(binding.path.mode & Markup::BindingPath::Mode::Read))
				continue;

			Refl::PropertyRef dst = {
			        .address = binding.value,
			        .type    = binding.type,
			};

			Refl::ConstPropertyRef src = {
			        .address = document(),
			        .type    = document()->bindings(),
			        .field   = binding.path.data(),
			        .fields  = binding.path.size(),
			};

			if (!Refl::Type::assign(dst, src, Refl::Property::Markup))
			{
				trinex_error(Log::Editor, "Failed to update binding");
			}
		}

		ImGui::PushID(this);
		{
			push_style();
			auto flags = on_begin_update();

			update_style_state(flags);

			if (flags & UpdateFlags::Readback)
			{
				for (auto& binding : m_bindings)
				{
					if (!(binding.path.mode & Markup::BindingPath::Mode::Write))
						continue;

					Refl::PropertyRef dst = {
					        .address = document(),
					        .type    = document()->bindings(),
					        .field   = binding.path.data(),
					        .fields  = binding.path.size(),
					};

					Refl::ConstValueRef src = {
					        .address = binding.value,
					        .type    = binding.type,
					};

					if (!Refl::Type::assign(dst, src, Refl::Property::Markup))
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
				on_end_update(flags);

			pop_style();
		}
		ImGui::PopID();

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

	void Element::push_style_var(ImGuiStyleVar var, f32 value)
	{
		ImGui::PushStyleVar(var, value);
	}

	void Element::push_style_var(ImGuiStyleVar var, const Vec2& value)
	{
		ImGui::PushStyleVar(var, ImVec2{value.x, value.y});
	}

	void Element::push_style_color(ImGuiCol color, const Vec4& value)
	{
		ImGui::PushStyleColor(color, ImVec4{value.x, value.y, value.z, value.w});
	}

	void Element::push_style_var(ImGuiStyleVar var, const ImVec2& value)
	{
		ImGui::PushStyleVar(var, value);
	}

	void Element::push_style_color(ImGuiCol color, const ImVec4& value)
	{
		ImGui::PushStyleColor(color, value);
	}

	Element& Element::push_style()
	{
		push_style_var(ImGuiStyleVar_ItemSpacing, spacing);
		push_style_var(ImGuiStyleVar_ItemInnerSpacing, inner_spacing);
		push_style_var(ImGuiStyleVar_Alpha, alpha);
		push_style_var(ImGuiStyleVar_IndentSpacing, indent);
		push_style_color(ImGuiCol_Text, text_color);
		push_style_color(ImGuiCol_Border, border_color);
		push_style_color(ImGuiCol_BorderShadow, border_shadow_color);
		return *this;
	}

	Element& Element::pop_style()
	{
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(4);
		return *this;
	}

	Element::UpdateFlags Element::on_begin_update()
	{
		return UpdateFlags::Default;
	}

	Element& Element::on_end_update(UpdateFlags flags)
	{
		return *this;
	}

	Refl::Type* Element::type() const
	{
		return Element::reflection();
	}

	Element::~Element()
	{
		clear();
	}
}// namespace Trinex::UI
