#pragma once
#include <Event/event.hpp>


namespace Engine
{
    class ENGINE_EXPORT EventSystemListenerID
    {
    private:
        EventType m_type;
        Identifier m_id;


    public:
        EventSystemListenerID();
        EventSystemListenerID(EventType, Identifier);
        copy_constructors_hpp(EventSystemListenerID);

        friend class EventSystem;
    };
}// namespace Engine
