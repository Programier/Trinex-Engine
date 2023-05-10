#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>

namespace Engine
{

    // extern template const Engine::Class* const ClassMetaData<class MakeHeaders*>::class_instance;
    class MakeHeaders : public CommandLet
    {
    public:
        int execute(int argc, char** argv) override
        {
            TextFileWriter* writer_ptr =
                    FileManager::root_file_manager()->create_text_file_writer("extern_metadata.hpp", true);
            if (!writer_ptr)
            {
                logger->error("MakeHeaders: Failed to create header 'extern_metadata.hpp'");
                return 1;
            }

            TextFileWriter& writer = *writer_ptr;

            writer << "#pragma once\n#include<Core/etl/metadata.hpp>\n\n";

            Map<String, Set<String>> classes;

            for (auto& pair : Class::classes())
            {
                auto pos = pair.first.find_last_of("::");
                if (pos != String::npos)
                {
                    String name_space = pair.first.substr(0, pos - 1);
                    pos += 1;
                    String class_name = pair.first.substr(pos, pair.first.length() - pos);

                    classes[name_space].insert(class_name);
                }
            }

            for (auto& pair : classes)
            {
                writer << "namespace " << pair.first << "\n{\n";
                for (auto& class_name : pair.second)
                {
                    writer << "\tclass " << class_name << ";\n";
                }
                writer << "}\n\n";
            }

            writer << "namespace Engine\n{\n";

            for (auto& pair : Class::classes())
            {
                String macros_name = Strings::format("{}_EXTERN_METADATA", Strings::replace_all(pair.first, "::", "_"));
                Strings::to_upper(macros_name);
                writer << "#ifndef " << macros_name << "\n";
                writer << "#define " << macros_name << "\n";
                writer << "\textern template const Engine::Class* const ClassMetaData<class " << pair.first
                       << "*>::class_instance;\n";

                writer << "#endif"
                       << "\n\n";
            }

            writer << "}\n";

            writer.close();
            delete writer_ptr;
            return 0;
        }
    };

    class Lol : public Engine::CommandLet
    {
        int execute(int argc, char** argv) override
        {
            logger->log("Hello World!");
            return 0;
        }
    };

    REGISTER_CLASS(Lol, Engine::CommandLet);
    REGISTER_CLASS(MakeHeaders, Engine::CommandLet);
}// namespace Engine
