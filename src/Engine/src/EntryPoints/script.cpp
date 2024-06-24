#include <Core/arguments.hpp>
#include <Core/class.hpp>
#include <Core/entry_point.hpp>
#include <Core/logger.hpp>
#include <ScriptEngine/script_module.hpp>
#include <fstream>
#include <ScriptEngine/script_function.hpp>


namespace Engine
{
    class ScriptExec : public EntryPoint
    {
        declare_class(ScriptExec, EntryPoint);

    public:
        static inline String read_content(std::istream& stream)
        {
            return String(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
        }

        static int_t exec_script(const String& source)
        {
            ScriptModule module("__TRINEX_SCRIPT_EXEC_MODULE__", ScriptModule::AlwaysCreate);
            if(module.add_script_section("Global", source.c_str(), source.length()) < 0)
            {
                error_log("ScriptExec", "Failed to add script section!");
                return -1;
            }

            if(module.build() < 0)
            {
                error_log("ScriptExec", "Failed to build module!");
                return -1;
            }

            ScriptFunction function = module.function_by_name("main");
            if(!function.is_valid())
            {
                error_log("ScriptExec", "Failed to get main function from script");
                return -1;
            }

            function.prepare();
            function.call();
            function.unbind_context();
            return 0;
        }

        int_t execute() override
        {
            auto file_argument = Arguments::find("file");

            if (file_argument == nullptr || file_argument->type != Arguments::Type::String)
            {
                error_log("ScriptExec", "Failed to get path to script file!");
                return -1;
            }

            std::ifstream file(file_argument->get<const String&>());
            if (!file.is_open())
            {
                error_log("ScriptExec", "Failed to open script file!");
                return -1;
            }

            String content = read_content(file);

            if (content.empty())
            {
                error_log("ScriptExec", "Script file is empty!");
                return -1;
            }


            return exec_script(content);
        }
    };

    implement_engine_class_default_init(ScriptExec, 0);
}// namespace Engine
