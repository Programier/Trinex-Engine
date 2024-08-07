#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/name.hpp>
#include <ScriptEngine/script_module.hpp>

namespace Engine
{
    class Script;

    class ENGINE_EXPORT ScriptFolder final
    {
    private:
        TreeMap<String, Script*> m_scripts;
        TreeMap<String, ScriptFolder*> m_folders;
        String m_name;
        ScriptFolder* m_parent = nullptr;

        ScriptFolder(const String& name, ScriptFolder* parent = nullptr);
        ~ScriptFolder();

    public:
        const TreeMap<String, Script*>& scripts() const;
        const TreeMap<String, ScriptFolder*>& sub_folders() const;
        ScriptFolder* find(const Path& path, bool create_if_not_exists = false);
        ScriptFolder* find(const Span<String>& path, bool create_if_not_exists = false);
        Script* find_script(const Path& script_path, bool create_if_not_exists = false);
        Script* find_script(const Span<String>& path, bool create_if_not_exists = false);

        ScriptFolder* parent() const;
        const String& name() const;
        Path path() const;

        friend class ScriptEngine;
        friend class Script;
    };

    class ENGINE_EXPORT Script final
    {
    private:
        ScriptModule m_module;
        String m_name;
        String m_code;
        ScriptFolder* m_folder;
        mutable bool m_is_dirty;

        Script(ScriptFolder* folder, const String& name);

    public:
        CallBacks<void(Script*)> on_build;

        const String& name() const;
        const String& code() const;
        Script& code(const String& code);

        bool is_dirty() const;
        Path path() const;
        bool load();
        bool save() const;
        bool build();
        ScriptModule module() const;

        ~Script();
        friend class ScriptFolder;
    };
}// namespace Engine
