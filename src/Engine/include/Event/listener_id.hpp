#pragma once
#include <Event/event.hpp>


namespace Engine
{
    class ENGINE_EXPORT EventSystemListenerID
    {
    private:
        EventType _M_type;
        Identifier _M_id;


    public:
        EventSystemListenerID();
        EventSystemListenerID(EventType, Identifier);
        copy_constructors_hpp(EventSystemListenerID);

        friend class EventSystem;
    };
}// namespace Engine
