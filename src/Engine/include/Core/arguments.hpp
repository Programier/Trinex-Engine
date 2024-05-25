#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>

namespace Engine
{
    class ENGINE_EXPORT Arguments
    {
    public:
        using StringType = String;
        using ArrayType  = Vector<StringType>;

        enum class Type
        {
            Define,
            String,
            Array,
        };

        struct ENGINE_EXPORT Argument {
            String name;
            Type type;
            Any data;

            Argument();
            Argument(const String& name);
            Argument(const String& name, const String& value);
            Argument(const String& name, const ArrayType& value);

            copy_constructors_hpp(Argument);

            template<typename T>
            FORCE_INLINE T get()
            {
                return data.cast<T>();
            }

            template<typename T>
            FORCE_INLINE T get() const
            {
                return data.cast<T>();
            }
        };

    private:
        static void push_argument(const char* name);
        static void push_array_argument(const String& name, const String& argument);
        static String parse_string_argument(const char* argument, size_t* out_pos = nullptr);

        static Map<String, Argument> m_arguments;

    public:
        static void init(int argc, const char** argv);
        static void clear();
        static const Map<String, Argument>& args();
        static Argument* find(const String& name);
        static void push_argument(const Argument& argument, bool override = false);
    };
}// namespace Engine
