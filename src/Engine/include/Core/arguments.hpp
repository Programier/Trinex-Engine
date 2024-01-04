#pragma once
#include <Core/engine_types.hpp>


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

        struct Argument {
            String name;
            Type type;
            Any data;
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
    };
}// namespace Engine
