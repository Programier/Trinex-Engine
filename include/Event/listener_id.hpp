#pragma once
#include <Event/event.hpp>


namespace Engine
{
	class ENGINE_EXPORT EventSystemListenerID
	{
	private:
		EventType m_type;
		Identifier m_id;
		bool m_is_valid;


	public:
		EventSystemListenerID();
		EventSystemListenerID(EventType, Identifier);
		copy_constructors_hpp(EventSystemListenerID);

		bool is_valid() const;

		friend class EventSystem;
	};
}// namespace Engine
