#pragma once
#include <Core/engine_types.hpp>


namespace Engine
{
    struct ENGINE_EXPORT ConfigManager {
        static bool load_from_text(const String& code);
        static bool load_from_file(const Path& filename);

        static bool set(const StringView& name, int value);
        static bool set(const StringView& name, float value);
        static bool set(const StringView& name, const StringView& value);

        // Array setter functions
        static bool set(const StringView& name, const Vector<bool>& value);
        static bool set(const StringView& name, const Vector<int>& value);
        static bool set(const StringView& name, const Vector<float>& value);
        static bool set(const StringView& name, const Vector<String>& value);


        static bool get_bool(const StringView& name);
        static int get_int(const StringView& name);
        static float get_float(const StringView& name);
        static String get_string(const StringView& name);
        static Path get_path(const StringView& name);
    };
}// namespace Engine
