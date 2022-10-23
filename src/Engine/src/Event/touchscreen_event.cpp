#include <Event/touchscreen_event.hpp>
#include <SDL_events.h>
#include <stdexcept>

#define min_value(a, b) (a < b ? a : b)

namespace Engine
{

    static unsigned int _M_num_fingers = 0;
    static unsigned int _M_prev_num_fingers = 0;
    static Finger _M_fingers[20];


    static Finger* private_get_finger(unsigned int index)
    {
        if (index > 19)
            return nullptr;
        return &_M_fingers[index];
    }

    void clear_touchscreen_events()
    {
        _M_prev_num_fingers = _M_num_fingers;
        unsigned int fingers = min_value(20, _M_num_fingers);
        for (unsigned int id = 0; id < fingers; id++) _M_fingers[id].offset = {0.f, 0.f};
    }

    void process_touchscreen_event(SDL_TouchFingerEvent& event)
    {
        _M_num_fingers = SDL_GetNumTouchFingers(event.touchId);
        Finger* finger = private_get_finger(event.fingerId);

        if (event.type == SDL_FINGERUP)
        {
            *finger = Finger();
            return;
        }

        finger->on_screen = true;
        finger->position.x = event.x;
        finger->position.y = event.y;
        finger->offset.x = event.dx;
        finger->offset.y = event.dy;
        finger->pressure = event.pressure;
    }

    ENGINE_EXPORT unsigned int TouchScreenEvent::fingers_count()
    {
        return _M_num_fingers;
    }

    ENGINE_EXPORT unsigned int TouchScreenEvent::prev_fingers_count()
    {
        return _M_prev_num_fingers;
    }

    ENGINE_EXPORT const Finger& TouchScreenEvent::get_finger(unsigned int index)
    {
        if (index > 19)
            throw std::runtime_error("Touchscreen: Finger index out of range");
        return _M_fingers[index];
    }
}// namespace Engine
