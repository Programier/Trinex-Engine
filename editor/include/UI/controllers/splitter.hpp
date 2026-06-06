#pragma once

#include <RmlUi/Core/EventListener.h>
#include <UI/rml.hpp>

namespace Trinex::UI
{
	class SplitterController final : public RMLController, public RML::EventListener
	{
		trinex_class(SplitterController, RMLController);

	private:
		enum class Orientation : u8
		{
			Horizontal,
			Vertical,
		};

		RML::Element* m_element  = nullptr;
		RML::Element* m_previous = nullptr;
		RML::Element* m_next     = nullptr;

		float m_start_axis      = 0.f;
		float m_previous_size   = 0.f;
		float m_next_size       = 0.f;
		float m_split_ratio     = 0.5f;
		float m_last_axis_size  = 0.f;
		float m_min_previous    = 120.f;
		float m_min_next        = 120.f;
		bool m_has_custom_size  = false;
		bool m_is_dragging      = false;
		Orientation m_direction = Orientation::Vertical;

		SplitterController& resolve_neighbors();
		SplitterController& apply_sizes(float previous, float next);
		float current_previous_size() const;
		float current_next_size() const;
		float current_axis_size() const;
		SplitterController& capture_split_state();
		float axis_from_event(RML::Event& event) const;

	public:
		void ProcessEvent(RML::Event& event) override;

		SplitterController& attach(RML::Element* element) override;
		SplitterController& update(RML::Element* element) override;
		SplitterController& deattach(RML::Element* element) override;
	};
}// namespace Trinex::UI
