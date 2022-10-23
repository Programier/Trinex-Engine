#pragma once
#include <string>
#include <Core/export.hpp>

namespace Engine
{
    STRUCT TextEvent
    {
        static ENGINE_EXPORT bool enable_text_writing;
        static ENGINE_EXPORT unsigned int last_symbol(bool reset = true);
        static ENGINE_EXPORT const std::string& get_text();
        static ENGINE_EXPORT const std::wstring& get_wide_text();
        static ENGINE_EXPORT const TextEvent& clear_text();
        static ENGINE_EXPORT const TextEvent& clear_wide_text();
        static ENGINE_EXPORT std::string get_clipboard_text();
        static ENGINE_EXPORT std::wstring get_clipboard_wtext();
    };
}
