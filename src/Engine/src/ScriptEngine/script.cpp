#include <Core/constants.hpp>
#include <Core/engine_config.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <ScriptEngine/script.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_module.hpp>

namespace Engine
{

    Script::Script(const Path& path) : _M_path(path)
    {}

    Script::~Script()
    {}

    const Path& Script::path() const
    {
        return _M_path;
    }

    Script& Script::replace_code(const char* new_code, size_t len, size_t offset)
    {
        ScriptModule::global().add_script_section(_M_path.string().c_str(), new_code, len, offset);
        return *this;
    }

    Script& Script::load()
    {
        FileManager manager(engine_config.scripts_dir);
        FileReader* reader = manager.create_file_reader(_M_path);

        if (reader)
        {
            Buffer buffer(reader->size(), 0);
            reader->read(buffer.data(), buffer.size());

            if (_M_path.string().ends_with(Constants::script_extension))
            {
                replace_code(reinterpret_cast<const char*>(buffer.data()), buffer.size(), 0);
            }
            else
            {
            }

            delete reader;
        }

        return *this;
    }

    class Script* ScriptEngine::new_script(const Path& path)
    {
        for (Script* script : _M_scripts)
        {
            if (script->path() == path)
                return script;
        }

        if (path.extension() == Constants::script_extension || path.extension() == Constants::script_byte_code_extension)
        {
            Script* script = new Script(path);
            _M_scripts.push_back(script);
            return script;
        }
        else
        {
            error_log("ScriptEngine", "Invalid script extension!");
            return nullptr;
        }
    }

    const Vector<class Script*>& ScriptEngine::scripts() const
    {
        return _M_scripts;
    }


    static void static_load_scripts(const Path& base, const Path& current, Set<Path>& scripts)
    {
        for (const auto& entry : FS::directory_iterator(current))
        {
            if (FS::is_regular_file(entry.path()))
            {
                if (entry.path().extension() == Constants::script_extension)
                {
                    Path diff = FS::relative(entry.path(), base);
                    scripts.insert(diff);
                    ScriptEngine::instance()->new_script(diff)->load();
                }
            }
            else if (FS::is_directory(entry.path()))
            {
                static_load_scripts(base, entry.path(), scripts);
            }
        }
    }

    void ScriptEngine::release_scripts()
    {
        for (Script* script : _M_scripts)
        {
            delete script;
        }

        _M_scripts.clear();
    }

    ScriptEngine& ScriptEngine::load_scripts()
    {
        release_scripts();
        ScriptModule::global().discard();

        Path path = FileManager::root_file_manager()->work_dir() / engine_config.scripts_dir;
        Set<Path> scripts;
        static_load_scripts(path, path, scripts);

        List<Script*> scripts_for_delete;
        ScriptModule::global().build();
        return *this;
    }
}// namespace Engine