#pragma once

#include <Event/keyboard_event.hpp>
#include <Event/mouse_event.hpp>
#include <Event/text_event.hpp>
#include <Event/touchscreen_event.hpp>
#include <Event/update_events.hpp>
#include <Core/callback.hpp>



namespace Engine
{
    struct ENGINE_EXPORT Event {
        static ENGINE_EXPORT KeyboardEvent keyboard;
        static ENGINE_EXPORT MouseEvent mouse;
        static ENGINE_EXPORT TextEvent text;
        static ENGINE_EXPORT TouchScreenEvent touchscreen;

        static ENGINE_EXPORT const Event& poll_events();
        static ENGINE_EXPORT const Event& wait_for_event();
        static ENGINE_EXPORT double diff_time();
        static ENGINE_EXPORT double time();

        // Callbacks
        static ENGINE_EXPORT CallBacks<void(void*)> sdl_callbacks;
        static ENGINE_EXPORT CallBacks<void(unsigned int)> on_sensor_update;
        static ENGINE_EXPORT CallBacks<void()> on_quit;
        static ENGINE_EXPORT CallBacks<void()> on_terminate;
        static ENGINE_EXPORT CallBacks<void()> on_resume;
        static ENGINE_EXPORT CallBacks<void()> on_pause;
        static ENGINE_EXPORT CallBacks<void()> on_low_memory;
        static ENGINE_EXPORT size_t frame_number();
    };
}// namespace Engine

//static KeyStatus get_key_status(const Key& key);
//static bool pressed(const Key& key);
