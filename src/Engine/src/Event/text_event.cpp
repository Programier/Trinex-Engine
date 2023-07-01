#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Event/text_event.hpp>
#include <SDL_clipboard.h>
#include <SDL_events.h>
#include <string>

namespace Engine
{
    static TextEvent _M_text_event;
    static unsigned int _M_last_symbol = 0;
    static String _M_text;

    ENGINE_EXPORT bool TextEvent::enable_text_writing = false;

    ENGINE_EXPORT unsigned int TextEvent::last_symbol(bool reset)
    {
        auto tmp = _M_last_symbol;
        if (reset)
            _M_last_symbol = 0;
        return tmp;
    }

    ENGINE_EXPORT const TextEvent& clear_text()
    {
        _M_text.clear();
        return _M_text_event;
    }

    ENGINE_EXPORT const String& TextEvent::text()
    {
        return _M_text;
    }

    ENGINE_EXPORT const TextEvent& TextEvent::clear_text()
    {
        _M_text.clear();
        return _M_text_event;
    }

    void process_text_event(SDL_TextInputEvent& event)
    {
        std::wstring str = Strings::to_wstring(event.text);
        _M_text += event.text;

        if ((_M_last_symbol = str[0]) && TextEvent::enable_text_writing)
        {
            _M_text.push_back(_M_last_symbol);
        }
    }


    void process_text_event(SDL_TextEditingEvent& event)
    {
        info_log("TextEvent", "%s\n", __PRETTY_FUNCTION__);
    }

    void process_text_event(SDL_TextEditingExtEvent& event)
    {
        info_log("TextEvent", "%s\n", __PRETTY_FUNCTION__);
    }

    ENGINE_EXPORT String TextEvent::clipboard_text()
    {
        return SDL_GetClipboardText();
    }

}// namespace Engine
