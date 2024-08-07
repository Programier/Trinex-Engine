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
    ScriptFolder::ScriptFolder(const String& name, ScriptFolder* parent) : m_name(name), m_parent(parent)
    {
        if (parent)
        {
            parent->m_folders.insert_or_assign(name, this);
        }
    }

    ScriptFolder::~ScriptFolder()
    {
        auto childs = std::move(m_folders);

        for (auto& child : childs)
        {
            delete child.second;
        }

        if (m_parent)
        {
            m_parent->m_folders.erase(m_name);
            m_parent = nullptr;
        }

        auto scripts = std::move(m_scripts);

        for (auto& script : scripts)
        {
            delete script.second;
        }
    }

    const TreeMap<String, Script*>& ScriptFolder::scripts() const
    {
        return m_scripts;
    }

    const TreeMap<String, ScriptFolder*>& ScriptFolder::sub_folders() const
    {
        return m_folders;
    }

    ScriptFolder* ScriptFolder::find(const Path& path, bool create_if_not_exists)
    {
        auto splited_path = path.split();
        return find(splited_path, create_if_not_exists);
    }

    ScriptFolder* ScriptFolder::find(const Span<String>& path, bool create_if_not_exists)
    {
        ScriptFolder* folder = this;

        for (auto& name : path)
        {
            if (name.empty())
                continue;

            if (name == "scripts:" && m_name == "scripts:")
                continue;

            auto it = folder->m_folders.find(name);

            if (it == folder->m_folders.end())
            {
                if (create_if_not_exists)
                {
                    folder = new ScriptFolder(name, folder);
                }
                else
                {
                    return nullptr;
                }
            }
            else
            {
                folder = it->second;
            }
        }

        return folder;
    }

    Script* ScriptFolder::find_script(const Path& script_path, bool create_if_not_exists)
    {
        auto splited_path = script_path.split();
        return find_script(splited_path, create_if_not_exists);
    }

    Script* ScriptFolder::find_script(const Span<String>& path, bool create_if_not_exists)
    {
        if (path.empty() || !path.back().ends_with(Constants::script_extension))
            return nullptr;

        if (ScriptFolder* folder = find(path.subspan(0, path.size() - 1), create_if_not_exists))
        {
            auto it = folder->m_scripts.find(path.back());

            if (it == folder->m_scripts.end())
            {
                if (create_if_not_exists)
                {
                    Script* new_script = new Script(folder, path.back());
                    folder->m_scripts.insert_or_assign(path.back(), new_script);
                    return new_script;
                }
                else
                {
                    return nullptr;
                }
            }

            return it->second;
        }

        return nullptr;
    }

    ScriptFolder* ScriptFolder::parent() const
    {
        return m_parent;
    }

    const String& ScriptFolder::name() const
    {
        return m_name;
    }

    Path ScriptFolder::path() const
    {
        if (m_parent)
        {
            return m_parent->path() / m_name;
        }
        return m_name;
    }

    Script::Script(ScriptFolder* folder, const String& name) : m_name(name), m_folder(folder), m_is_dirty(false)
    {}

    Script::~Script()
    {
        m_module.discard();
        m_folder->m_scripts.erase(m_name);
    }

    const String& Script::name() const
    {
        return m_name;
    }

    const String& Script::code() const
    {
        return m_code;
    }

    Script& Script::code(const String& code)
    {
        m_code     = code;
        m_is_dirty = true;
        return *this;
    }

    bool Script::is_dirty() const
    {
        return m_is_dirty;
    }

    Path Script::path() const
    {
        return m_folder->path() / m_name;
    }

    bool Script::load()
    {
        FileReader reader(path());

        if (reader.is_open())
        {
            String new_code(reader.size(), 0);

            if (new_code.size() > 0 && reader.read(reinterpret_cast<byte*>(new_code.data()), new_code.size()))
            {
                m_code = std::move(new_code);
                return true;
            }
        }

        return false;
    }

    bool Script::save() const
    {
        FileWriter writer(path());

        if (writer.is_open())
        {
            if (writer.write(reinterpret_cast<const byte*>(m_code.c_str()), m_code.size()))
            {
                m_is_dirty = false;
                return true;
            }
        }

        return false;
    }

    bool Script::build()
    {
        if (m_module.is_valid())
            m_module.discard();

        m_module = ScriptModule(path().c_str());
        m_module.add_script_section("Code", m_code.data(), m_code.size());

        bool result = m_module.build();

        if (result)
        {
            on_build(this);
        }

        return result;
    }

    ScriptModule Script::module() const
    {
        return m_module;
    }

    static void static_load_scripts(ScriptFolder* folder)
    {
        auto fs = rootfs();
        for (const auto& entry : VFS::DirectoryIterator(folder->path()))
        {
            if (rootfs()->is_file(entry))
            {
                if (entry.extension() == Constants::script_extension)
                {
                    auto script = folder->find_script(entry.filename(), true);
                    if (script->load())
                        script->build();
                }
            }
            else if (fs->is_dir(entry))
            {
                static_load_scripts(folder->find(entry.filename(), true));
            }
        }
    }

    ScriptEngine& ScriptEngine::load_scripts()
    {
        static_load_scripts(m_script_folder);
        return instance();
    }
}// namespace Engine
