#include <Core/constants.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/directory_iterator.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Engine/project.hpp>
#include <ScriptEngine/script.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_module.hpp>

namespace Engine
{

    Script::Script(const Path& path) : m_path(path)
    {}

    Script::~Script()
    {}

    const Path& Script::path() const
    {
        return m_path;
    }

    Script& Script::replace_code(const char* new_code, size_t len, size_t offset)
    {
        ScriptModule::global().add_script_section(m_path.str().c_str(), new_code, len, offset);
        return *this;
    }

    Script& Script::load()
    {
        FileReader reader(Path(Project::scripts_dir) / m_path);
        if (reader.is_open())
        {
            Buffer buffer(reader.size(), 0);
            reader.read(buffer.data(), buffer.size());

            if (m_path.str().ends_with(Constants::script_extension))
            {
                replace_code(reinterpret_cast<const char*>(buffer.data()), buffer.size(), 0);
            }
            else
            {
                throw not_implemented;
            }
        }

        return *this;
    }

    class Script* ScriptEngine::new_script(const Path& path)
    {
        for (Script* script : m_scripts)
        {
            if (script->path() == path)
                return script;
        }

        if (path.extension() == Constants::script_extension || path.extension() == Constants::script_byte_code_extension)
        {
            Script* script = new Script(path);
            m_scripts.push_back(script);
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
        return m_scripts;
    }


    static void static_load_scripts(const Path& base, const Path& current, Set<Path>& scripts)
    {
        auto fs = rootfs();
        for (const auto& entry : VFS::DirectoryIterator(current))
        {
            if (rootfs()->is_file(entry))
            {
                if (entry.extension() == Constants::script_extension)
                {
                    Path relative = entry.relative(base);
                    scripts.insert(relative);
                    ScriptEngine::instance()->new_script(relative)->load();
                }
            }
            else if (fs->is_dir(entry))
            {
                static_load_scripts(base, entry, scripts);
            }
        }
    }

    void ScriptEngine::release_scripts()
    {
        for (Script* script : m_scripts)
        {
            delete script;
        }

        m_scripts.clear();
    }

    ScriptEngine& ScriptEngine::load_scripts()
    {
        release_scripts();
        ScriptModule::global().discard();

        Path path = Project::scripts_dir;
        Set<Path> scripts;
        static_load_scripts(path, path, scripts);

        List<Script*> scripts_for_delete;
        ScriptModule::global().build();
        return *this;
    }
}// namespace Engine
