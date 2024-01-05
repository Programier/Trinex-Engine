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
                return std::any_cast<T>(data);
            }

            template<typename T>
            FORCE_INLINE T get() const
            {
                return std::any_cast<T>(data);
            }
        };

    private:
        void push_argument(const char* name);
        void push_array_argument(const String& name, const String& argument);
        String parse_string_argument(const char* argument, size_t* out_pos = nullptr);
        Map<String, Argument> _M_arguments;

    public:
        Arguments& init(int argc, char** argv);
        Arguments& clear();
        const Map<String, Argument>& args() const;
        const Argument* find(const String& name) const;
        Argument* find(const String& name);
        Arguments& push_argument(const Argument& argument, bool override = false);
    };
}// namespace Engine
