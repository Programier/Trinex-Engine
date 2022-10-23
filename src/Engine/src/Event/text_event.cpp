#include <Core/logger.hpp>
#include <Event/text_event.hpp>
#include <SDL_clipboard.h>
#include <SDL_events.h>
#include <codecvt>
#include <locale>
#include <string>

namespace Engine
{
    static TextEvent _M_text_event;
    static unsigned int _M_last_symbol = 0;
    static std::string _M_text;
    static std::wstring _M_wtext;
    ENGINE_EXPORT bool TextEvent::enable_text_writing = false;

    ENGINE_EXPORT unsigned int TextEvent::last_symbol(bool reset)
    {
        auto tmp = _M_last_symbol;
        if (reset)
            _M_last_symbol = 0;
        return tmp;
    }

    ENGINE_EXPORT const std::string& TextEvent::get_text()
    {
        return _M_text;
    }

    ENGINE_EXPORT const TextEvent& clear_text()
    {
        _M_text.clear();
        return _M_text_event;
    }

    ENGINE_EXPORT const std::wstring& TextEvent::get_wide_text()
    {
        return _M_wtext;
    }

    ENGINE_EXPORT const TextEvent& TextEvent::clear_wide_text()
    {
        _M_wtext.clear();
        return _M_text_event;
    }

    static std::wstring to_wstring(const std::string& str)
    {
        static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convertor;
        return convertor.from_bytes(str);
    }

    void process_text_event(SDL_TextInputEvent& event)
    {
        std::string str(event.text);
        if ((_M_last_symbol = to_wstring(str)[0]) && TextEvent::enable_text_writing)
        {
            _M_text += str;
            _M_wtext.push_back(_M_last_symbol);
        }
    }


    void process_text_event(SDL_TextEditingEvent& event)
    {
        logger->log("%s\n", __PRETTY_FUNCTION__);
    }

    void process_text_event(SDL_TextEditingExtEvent& event)
    {
        logger->log("%s\n", __PRETTY_FUNCTION__);
    }

    ENGINE_EXPORT std::string TextEvent::get_clipboard_text()
    {
        return SDL_GetClipboardText();
    }

    ENGINE_EXPORT std::wstring TextEvent::get_clipboard_wtext()
    {
        return to_wstring(get_clipboard_text());
    }
}// namespace Engine
