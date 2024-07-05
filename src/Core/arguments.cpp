#include <Core/arguments.hpp>
#include <cstring>
#include <regex>

namespace Engine
{
    Map<String, Arguments::Argument> Arguments::m_arguments;
    int_t Arguments::m_argc        = 0;
    const char** Arguments::m_argv = nullptr;

    Arguments::Argument::Argument() = default;

    Arguments::Argument::Argument(const String& name) : Argument()
    {
        this->name = name;
        type       = Type::Define;
    }

    Arguments::Argument::Argument(const String& name, const String& value) : Argument(name, StringView(value))
    {}

    Arguments::Argument::Argument(const String& name, const StringView& value) : Argument(name)
    {
        data = String(value);
        type = Type::String;
    }

    Arguments::Argument::Argument(const String& name, const char* value) : Argument(name, StringView(value))
    {}

    Arguments::Argument::Argument(const String& name, const ArrayType& value) : Argument(name)
    {
        data = value;
        type = Type::Array;
    }

    default_copy_constructors_scoped_cpp(Arguments, Argument);


    void Arguments::init(int argc, const char** argv)
    {
        m_argc = argc;
        m_argv = argv;

        for (int i = 0; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                push_argument(argv[i] + 1);
            }
        }
    }

    int_t Arguments::argc()
    {
        return m_argc;
    }

    const char** Arguments::argv()
    {
        return m_argv;
    }

    String Arguments::parse_string_argument(const char* argument, size_t* out_pos)
    {
        char end_char = '\0';
        size_t start  = 0;

        if (argument[0] == '\'' || argument[0] == '"')
        {
            end_char = argument[0];
            start    = 1;
        }

        size_t end = start;

        while (argument[end] != end_char && argument[end] != '\0')
        {
            ++end;
        }

        if (argument[end] != end_char)
        {
            start = 0;
        }

        if (out_pos)
        {
            *out_pos = end;
        }

        return String(argument + start, end);
    }


    void Arguments::push_array_argument(const String& name, const String& argument)
    {
        Vector<String> data = {};

        static std::regex regex("\\b(?:'([^']+)'|\"([^\"]+)\"|([^,]+))\\b");

        std::sregex_iterator iter(argument.begin(), argument.end(), regex);
        std::sregex_iterator end;

        while (iter != end)
        {
            String match = (*iter).str();
            data.push_back(match);
            ++iter;
        }

        auto& arg = m_arguments[name];
        arg.name  = std::move(name);
        arg.type  = Type::Array;
        arg.data  = data;
    }


    void Arguments::push_argument(const char* name)
    {
        size_t pos = 0;
        size_t len = std::strlen(name);

        for (pos = 0; pos < len && name[pos] != '='; ++pos)
            ;

        size_t arg_pos = pos + 1;
        for (; arg_pos < len && name[arg_pos] == ' '; ++arg_pos)
            ;

        String arg_name = String(name, pos);

        if (pos >= len - 1 && arg_pos >= len)
        {
            auto& argument = m_arguments[arg_name];
            argument.name  = std::move(arg_name);
            argument.type  = Type::Define;
            return;
        }

        if (name[arg_pos] != '{')
        {
            auto& argument = m_arguments[arg_name];
            argument.name  = std::move(arg_name);
            argument.type  = Type::String;
            argument.data  = parse_string_argument(name + arg_pos);
            return;
        }
        else
        {
            push_array_argument(arg_name, String(name + arg_pos));
        }
    }

    void Arguments::clear()
    {
        m_arguments.clear();
    }

    const Map<String, Arguments::Argument>& Arguments::args()
    {
        return m_arguments;
    }

    Arguments::Argument* Arguments::find(const String& name)
    {
        auto it = m_arguments.find(name);
        if (it == m_arguments.end())
            return nullptr;
        return &it->second;
    }

    void Arguments::push_argument(const Argument& argument, bool override)
    {
        Argument* arg = find(argument.name);
        if (arg)
        {
            if (override)
            {
                arg->data = argument.data;
                arg->type = argument.type;
            }
            return;
        }

        m_arguments[argument.name] = argument;
        return;
    }
}// namespace Engine
