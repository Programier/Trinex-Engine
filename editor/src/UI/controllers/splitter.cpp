#include <Core/math/math.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <RmlUi/Core/ElementDocument.h>
#include <UI/controllers/splitter.hpp>

namespace Trinex::UI
{
	namespace
	{
		static void normalize_pane_sizes(float total_size, float preferred_previous, float min_previous, float min_next,
		                                 float& previous, float& next)
		{
			total_size = Math::max(0.f, total_size);

			if (total_size <= 0.f)
			{
				previous = 0.f;
				next     = 0.f;
				return;
			}

			const float min_total = min_previous + min_next;
			if (total_size <= min_total)
			{
				const float ratio = (min_total > 0.f) ? Math::clamp(preferred_previous / min_total, 0.f, 1.f) : 0.5f;
				previous          = total_size * ratio;
				next              = total_size - previous;
				return;
			}

			previous = Math::clamp(preferred_previous, min_previous, total_size - min_next);
			next     = total_size - previous;
		}
	}// namespace

	trinex_implement_class(Trinex::UI::SplitterController, 0) {}

	SplitterController& SplitterController::resolve_neighbors()
	{
		m_previous = m_element ? m_element->GetPreviousSibling() : nullptr;
		m_next     = m_element ? m_element->GetNextSibling() : nullptr;
		return *this;
	}

	float SplitterController::current_previous_size() const
	{
		if (m_previous == nullptr)
			return 0.f;

		return (m_direction == Orientation::Vertical) ? m_previous->GetOffsetWidth() : m_previous->GetOffsetHeight();
	}

	float SplitterController::current_next_size() const
	{
		if (m_next == nullptr)
			return 0.f;

		return (m_direction == Orientation::Vertical) ? m_next->GetOffsetWidth() : m_next->GetOffsetHeight();
	}

	float SplitterController::current_axis_size() const
	{
		if (m_element == nullptr)
			return 0.f;

		RML::Element* parent = m_element->GetParentNode();
		if (parent == nullptr)
			return 0.f;

		return (m_direction == Orientation::Vertical) ? parent->GetOffsetWidth() : parent->GetOffsetHeight();
	}

	SplitterController& SplitterController::capture_split_state()
	{
		const float previous = current_previous_size();
		const float next     = current_next_size();
		const float total    = previous + next;

		if (total > 0.f)
		{
			m_split_ratio = previous / total;
		}

		m_last_axis_size = current_axis_size();
		return *this;
	}

	SplitterController& SplitterController::apply_sizes(float previous, float next)
	{
		if (m_previous == nullptr || m_next == nullptr)
			return *this;

		const char* axis_property = m_direction == Orientation::Vertical ? "width" : "height";

		m_previous->SetProperty("flex", "0 0 auto");
		m_next->SetProperty("flex", "0 0 auto");
		m_previous->SetProperty(axis_property, Strings::format("{:.0f}px", previous));
		m_next->SetProperty(axis_property, Strings::format("{:.0f}px", next));
		m_has_custom_size = true;
		capture_split_state();
		return *this;
	}

	float SplitterController::axis_from_event(RML::Event& event) const
	{
		const auto position = event.GetUnprojectedMouseScreenPos();
		return m_direction == Orientation::Vertical ? position.x : position.y;
	}

	void SplitterController::ProcessEvent(RML::Event& event)
	{
		if (m_element == nullptr)
			return;

		if (event == "mousedown")
		{
			resolve_neighbors();

			if (m_previous == nullptr || m_next == nullptr)
				return;

			m_is_dragging = true;
			m_start_axis  = axis_from_event(event);
			m_previous_size =
			        (m_direction == Orientation::Vertical) ? m_previous->GetOffsetWidth() : m_previous->GetOffsetHeight();
			m_next_size = (m_direction == Orientation::Vertical) ? m_next->GetOffsetWidth() : m_next->GetOffsetHeight();
			event.StopPropagation();
			return;
		}

		if (!m_is_dragging)
			return;

		if (event == "mouseup")
		{
			m_is_dragging = false;
			event.StopPropagation();
			return;
		}

		if (event == "mousemove")
		{
			const float delta              = axis_from_event(event) - m_start_axis;
			const float total_size         = m_previous_size + m_next_size;
			const float preferred_previous = m_previous_size + delta;
			float previous                 = 0.f;
			float next                     = 0.f;

			normalize_pane_sizes(total_size, preferred_previous, m_min_previous, m_min_next, previous, next);

			apply_sizes(previous, next);
			event.StopPropagation();
		}
	}

	SplitterController& SplitterController::attach(RML::Element* element)
	{
		Super::attach(element);

		m_element     = element;
		m_is_dragging = false;
		m_direction   = element->Matches(".trx-splitter--horizontal") ? Orientation::Horizontal : Orientation::Vertical;

		m_element->AddEventListener("mousedown", this);

		if (auto* document = m_element->GetOwnerDocument())
		{
			document->AddEventListener("mousemove", this, true);
			document->AddEventListener("mouseup", this, true);
		}

		resolve_neighbors();
		capture_split_state();
		return *this;
	}

	SplitterController& SplitterController::update(RML::Element* element)
	{
		Super::update(element);

		if (m_element != element)
			return *this;

		if (m_is_dragging)
			return *this;

		if (!m_has_custom_size)
			return *this;

		resolve_neighbors();

		if (m_previous == nullptr || m_next == nullptr)
			return *this;

		const float axis_size = current_axis_size();
		if (axis_size <= 0.f || axis_size == m_last_axis_size)
			return *this;

		const float delta              = axis_size - m_last_axis_size;
		const float total_size         = Math::max(0.f, current_previous_size() + current_next_size() + delta);
		const float preferred_previous = total_size * m_split_ratio;
		float previous                 = 0.f;
		float next                     = 0.f;

		normalize_pane_sizes(total_size, preferred_previous, m_min_previous, m_min_next, previous, next);

		apply_sizes(previous, next);
		return *this;
	}

	SplitterController& SplitterController::deattach(RML::Element* element)
	{
		if (m_element)
		{
			m_element->RemoveEventListener("mousedown", this);

			if (auto* document = m_element->GetOwnerDocument())
			{
				document->RemoveEventListener("mousemove", this, true);
				document->RemoveEventListener("mouseup", this, true);
			}
		}

		m_element         = nullptr;
		m_previous        = nullptr;
		m_next            = nullptr;
		m_is_dragging     = false;
		m_has_custom_size = false;
		m_start_axis      = 0.f;
		m_previous_size   = 0.f;
		m_next_size       = 0.f;
		m_split_ratio     = 0.5f;
		m_last_axis_size  = 0.f;

		Super::deattach(element);
		return *this;
	}
}// namespace Trinex::UI
