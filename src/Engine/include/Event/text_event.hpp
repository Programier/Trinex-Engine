#pragma once
#include <string>
#include <Core/export.hpp>
#include <Core/engine_types.hpp>

namespace Engine
{
    struct ENGINE_EXPORT TextEvent
    {
        static ENGINE_EXPORT bool enable_text_writing;
        static ENGINE_EXPORT uint_t last_symbol(bool reset = true);
        static ENGINE_EXPORT const String& text();
        static ENGINE_EXPORT const TextEvent& clear_text();
        static ENGINE_EXPORT String clipboard_text();
    };
}
