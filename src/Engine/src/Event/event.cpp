#include <Core/engine_loading_controllers.hpp>
#include <Event/event.hpp>
#include <Event/event_data.hpp>


namespace Engine
{
    void Event::add_to_id(int_t value, int_t offset)
    {
        _M_ID |= static_cast<ID>(value + 1) << offset;
    }

    Event::Event(EventType type) : _M_any({}), _M_ID(0), _M_type(type)
    {
        add_to_id(static_cast<int_t>(type), sizeof(ID) * 4);
    }

    EventType Event::type() const
    {
        return _M_type;
    }

    bool Event::operator==(const Event& e)
    {
        return _M_ID == e._M_ID;
    }

    bool Event::operator!=(const Event& e)
    {
        return _M_ID != e._M_ID;
    }

    Event::ID Event::id() const
    {
        return _M_ID;
    }

    Event::ID Event::base_id() const
    {
        constexpr ID mask = (~static_cast<ID>(0)) << (sizeof(ID) * 4);
        return id() & mask;
    }

    Event::ID Event::child_id() const
    {
        constexpr ID mask = (~static_cast<ID>(0)) >> (sizeof(ID) * 4);
        return id() & mask;
    }


    template<typename... Args>
    static Event* static_create_event(EventType type, const Args&... args)
    {
        info_log("Event", "%s", __PRETTY_FUNCTION__);
        return new Event(type, args...);
    }
}// namespace Engine
