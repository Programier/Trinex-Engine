#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_lua.hpp>
#include <Event/event.hpp>
#include <Event/text_event.hpp>

namespace Engine
{


    static void initialize_enums(Lua::Namespace& n)
    {
        n.new_enum<Key>("Key", {{"Unknown", Key::Unknown},
                                {"Space", Key::Space},
                                {"Apostrophe", Key::Apostrophe},
                                {"Comma", Key::Comma},
                                {"Minus", Key::Minus},
                                {"Period", Key::Period},
                                {"Slash", Key::Slash},
                                {"Num0", Key::Num0},
                                {"Num1", Key::Num1},
                                {"Num2", Key::Num2},
                                {"Num3", Key::Num3},
                                {"Num4", Key::Num4},
                                {"Num5", Key::Num5},
                                {"Num6", Key::Num6},
                                {"Num7", Key::Num7},
                                {"Num8", Key::Num8},
                                {"Num9", Key::Num9},
                                {"Semicolon", Key::Semicolon},
                                {"Equal", Key::Equal},
                                {"A", Key::A},
                                {"B", Key::B},
                                {"C", Key::C},
                                {"D", Key::D},
                                {"E", Key::E},
                                {"F", Key::F},
                                {"G", Key::G},
                                {"H", Key::H},
                                {"I", Key::I},
                                {"J", Key::J},
                                {"K", Key::K},
                                {"L", Key::L},
                                {"M", Key::M},
                                {"N", Key::N},
                                {"O", Key::O},
                                {"P", Key::P},
                                {"Q", Key::Q},
                                {"R", Key::R},
                                {"S", Key::S},
                                {"T", Key::T},
                                {"U", Key::U},
                                {"V", Key::V},
                                {"W", Key::W},
                                {"X", Key::X},
                                {"Y", Key::Y},
                                {"Z", Key::Z},
                                {"LeftBracket", Key::LeftBracket},
                                {"Backslash", Key::Backslash},
                                {"RightBracket", Key::RightBracket},
                                {"GraveAccent", Key::GraveAccent},
                                {"WWW", Key::Www},
                                {"Escape", Key::Escape},
                                {"Enter", Key::Enter},
                                {"Tab", Key::Tab},
                                {"Backspace", Key::Backspace},
                                {"Insert", Key::Insert},
                                {"Delete", Key::Delete},
                                {"Right", Key::Right},
                                {"Left", Key::Left},
                                {"Down", Key::Down},
                                {"Up", Key::Up},
                                {"PageUp", Key::PageUp},
                                {"PageDown", Key::PageDown},
                                {"Home", Key::Home},
                                {"End", Key::End},
                                {"CapsLock", Key::CapsLock},
                                {"ScrollLock", Key::ScrollLock},
                                {"NumLock", Key::NumLock},
                                {"PrintScreen", Key::PrintScreen},
                                {"Pause", Key::Pause},
                                {"F1", Key::F1},
                                {"F2", Key::F2},
                                {"F3", Key::F3},
                                {"F4", Key::F4},
                                {"F5", Key::F5},
                                {"F6", Key::F6},
                                {"F7", Key::F7},
                                {"F8", Key::F8},
                                {"F9", Key::F9},
                                {"F10", Key::F10},
                                {"F11", Key::F11},
                                {"F12", Key::F12},
                                {"F13", Key::F13},
                                {"F14", Key::F14},
                                {"F15", Key::F15},
                                {"F16", Key::F16},
                                {"F17", Key::F17},
                                {"F18", Key::F18},
                                {"F19", Key::F19},
                                {"F20", Key::F20},
                                {"F21", Key::F21},
                                {"F22", Key::F22},
                                {"F23", Key::F23},
                                {"F24", Key::F24},
                                {"Kp0", Key::Kp0},
                                {"Kp1", Key::Kp1},
                                {"Kp2", Key::Kp2},
                                {"Kp3", Key::Kp3},
                                {"Kp4", Key::Kp4},
                                {"Kp5", Key::Kp5},
                                {"Kp6", Key::Kp6},
                                {"Kp7", Key::Kp7},
                                {"Kp8", Key::Kp8},
                                {"Kp9", Key::Kp9},
                                {"KpDecimal", Key::KpDecimal},
                                {"KpDivide", Key::KpDivide},
                                {"KpMultiply", Key::KpMultiply},
                                {"KpSubtract", Key::KpSubtract},
                                {"KpAdd", Key::KpAdd},
                                {"KpEnter", Key::KpEnter},
                                {"KpEqual", Key::KpEqual},
                                {"LeftShift", Key::LeftShift},
                                {"LeftControl", Key::LeftControl},
                                {"LeftAlt", Key::LeftAlt},
                                {"LeftSuper", Key::LeftSuper},
                                {"RightShift", Key::RightShift},
                                {"RightControl", Key::RightControl},
                                {"RightAlt", Key::RightAlt},
                                {"RightSuper", Key::RightSuper},
                                {"Menu", Key::Menu},
                                {"MouseLeft", Key::MouseLeft},
                                {"MouseRight", Key::MouseRight},
                                {"MouseMiddle", Key::MouseMiddle}});

        n.new_enum<KeyStatus>("KeyStatus", {{"Released", KeyStatus::Released},
                                            {"JustReleased", KeyStatus::JustReleased},
                                            {"JustPressed", KeyStatus::JustPressed},
                                            {"Pressed", KeyStatus::Pressed},
                                            {"Repeat", KeyStatus::Repeat}});
    }


    static void initialize_keyboard()
    {
        Lua::Class<KeyboardEvent> _class = Lua::Interpretter::lua_class_of<KeyboardEvent>("Engine::KeyboardEvent");

        _class.set("just_released", Lua::overload(static_cast<Key (*)()>(KeyboardEvent::just_released),
                                                  static_cast<bool (*)(Key key)>(KeyboardEvent::just_released)));
        _class.set("last_pressed", Lua::overload(static_cast<Key (*)()>(KeyboardEvent::last_pressed),
                                                 static_cast<bool (*)(Key key)>(KeyboardEvent::last_pressed)));
        _class.set("just_pressed", Lua::overload(static_cast<Key (*)()>(KeyboardEvent::just_pressed),
                                                 static_cast<bool (*)(Key key)>(KeyboardEvent::just_pressed)));
        _class.set("last_symbol",
                   Lua::overload(KeyboardEvent::last_symbol, []() { return KeyboardEvent::last_symbol(); }));

        _class.set("get_key_status", KeyboardEvent::get_key_status);
        _class.set("pressed", static_cast<bool (*)(const Key&)>(&KeyboardEvent::pressed));
        _class.set("just_evented_keys", KeyboardEvent::just_evented_keys);
        _class.set("push_event", KeyboardEvent::push_event);
    }


    static void initialize_mouse()
    {
        Lua::Class<MouseEvent> _class = Lua::Interpretter::lua_class_of<MouseEvent>("Engine::MouseEvent");

        _class.set("just_released", Lua::overload(static_cast<Key (*)()>(MouseEvent::just_released),
                                                  static_cast<bool (*)(Key key)>(MouseEvent::just_released)));
        _class.set("last_pressed", Lua::overload(static_cast<Key (*)()>(MouseEvent::last_pressed),
                                                 static_cast<bool (*)(Key key)>(MouseEvent::last_pressed)));
        _class.set("just_pressed", Lua::overload(static_cast<Key (*)()>(MouseEvent::just_pressed),
                                                 static_cast<bool (*)(Key key)>(MouseEvent::just_pressed)));

        _class.set("position", Lua::overload(static_cast<const Point2D& (*) ()>(MouseEvent::position),
                                             static_cast<void (*)(const Point2D&)>(MouseEvent::position)));

        _class.set("offset", MouseEvent::offset);
        _class.set("scroll_offset", MouseEvent::scroll_offset);
        _class.set("get_key_status", MouseEvent::get_key_status);
        _class.set("pressed", static_cast<bool (*)(const Key&)>(&MouseEvent::pressed));
        _class.set("just_evented_keys", MouseEvent::just_evented_keys);

        _class.set("relative_mode", Lua::overload(static_cast<bool (*)()>(MouseEvent::relative_mode),
                                                  static_cast<void (*)(bool)>(MouseEvent::relative_mode)));
    }


    static void initialize_text()
    {
        Lua::Class<TextEvent> _class = Lua::Interpretter::lua_class_of<TextEvent>("Engine::TextEvent");

        _class.set("last_symbol", Lua::overload(&TextEvent::last_symbol, []() { return TextEvent::last_symbol(); }));
        _class.set("text", &TextEvent::text);
        _class.set("clear_text", &TextEvent::clear_text);
        _class.set("clipboard_text", &TextEvent::clipboard_text);
    }

    static void initialize_touchscreen()
    {
        {
            Lua::Class<Finger> _class = Lua::Interpretter::lua_class_of<Finger>("Engine::Finger");
            _class.set("on_screen", &Finger::on_screen);
            _class.set("position", &Finger::position);
            _class.set("offset", &Finger::offset);
            _class.set("pressure", &Finger::pressure);
        }

        {
            Lua::Class<TouchScreenEvent> _class =
                    Lua::Interpretter::lua_class_of<TouchScreenEvent>("Engine::TouchScreenEvent");
            _class.set("fingers_count", &TouchScreenEvent::fingers_count);
            _class.set("prev_fingers_count", &TouchScreenEvent::prev_fingers_count);
            _class.set("get_finger", &TouchScreenEvent::get_finger);
        }
    }

    static void initialize_event()
    {
        Lua::Class<Event> _class = Lua::Interpretter::lua_class_of<Event>("Engine::Event");
        _class.set("poll_events", &Event::poll_events);
        _class.set("wait_for_event", &Event::wait_for_event);
        _class.set("diff_time", &Event::diff_time);
        _class.set("time", &Event::time);
        _class.set("frame_number", &Event::frame_number);
    }


#define set_value(a) _namespace.set(#a, a)
    static void on_init()
    {

        Lua::Namespace _namespace = Lua::Interpretter::namespace_of("Engine::");
        initialize_enums(_namespace);

        initialize_keyboard();
        initialize_mouse();
        initialize_text();
        initialize_touchscreen();
        initialize_event();

        set_value(to_key);
        set_value(to_SDL_scancode);
        set_value(to_character);
        set_value(get_key_name);
        set_value(key_count);
    }

    static InitializeController init(on_init);
}// namespace Engine
