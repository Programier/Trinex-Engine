#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/name.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_type_info.hpp>

namespace Engine
{
    class Script;

    class ENGINE_EXPORT ScriptFolder final
    {
    private:
        TreeMap<String, Script*> m_scripts;
        TreeMap<String, ScriptFolder*> m_folders;
        Path m_path;
        String m_name;
        ScriptFolder* m_parent = nullptr;

        ScriptFolder(const String& name, ScriptFolder* parent = nullptr);
        ~ScriptFolder();

        ScriptFolder& on_path_changed();

    public:
        const TreeMap<String, Script*>& scripts() const;
        const TreeMap<String, ScriptFolder*>& sub_folders() const;
        ScriptFolder* find(const Path& path, bool create_if_not_exists = false);
        ScriptFolder* find(const Span<String>& path, bool create_if_not_exists = false);
        Script* find_script(const Path& script_path, bool create_if_not_exists = false);
        Script* find_script(const Span<String>& path, bool create_if_not_exists = false);

        ScriptFolder* parent() const;
        const String& name() const;
        const Path& path() const;

        friend class ScriptEngine;
        friend class Script;
    };

    class ENGINE_EXPORT Script final
    {
    public:
        struct ENGINE_EXPORT ClassMetadata {
            ScriptTypeInfo type_info;
            TreeSet<String> class_metadata;
            TreeMap<int_t, TreeSet<String>> func_metadata_map;
            TreeMap<int_t, TreeSet<String>> prop_metadata_map;

            const TreeSet<String>& metadata_for_func(const ScriptFunction& func) const;
            const TreeSet<String>& metadata_for_var(uint_t prop_index) const;
        };

    private:
        class Builder;

        Path m_path;
        ScriptModule m_module;
        String m_name;
        String m_code;
        ScriptFolder* m_folder;
        Set<class Class*> m_classes;

        // Metadata info
        TreeMap<int_t, TreeSet<String>> m_func_metadata_map;
        TreeMap<int_t, TreeSet<String>> m_var_metadata_map;
        TreeMap<int_t, ClassMetadata> m_class_metadata_map;

        mutable bool m_is_dirty;

        Script(ScriptFolder* folder, const String& name);

        Script& load_metadata(Builder& builder);
		bool load_classes(asITypeInfo* info);
        Script& load_classes();
        Script& unload_classes();
        Script& on_path_changed();

    public:
        CallBacks<void(Script*)> on_build;
        CallBacks<void(Script*)> on_discard;
        CallBacks<void(Script*)> on_exception;

        const String& name() const;
        const String& code() const;
        Script& code(const String& code);

        bool is_dirty() const;
        const Path& path() const;
        bool load();
        bool save() const;
        bool build(bool exception_on_error = true);

        // Metadata
        const TreeMap<int_t, TreeSet<String>>& func_metadata_map() const;
        const TreeMap<int_t, TreeSet<String>>& var_metadata_map() const;
        const TreeMap<int_t, ClassMetadata>& class_metadata_map() const;

        const TreeSet<String>& metadata_for_func(const ScriptFunction& func) const;
        const TreeSet<String>& metadata_for_var(uint_t var_index) const;
        const ClassMetadata& metadata_for_class(const ScriptTypeInfo& info) const;

        // Functions
        uint_t functions_count() const;
        ScriptFunction function_by_index(uint_t index) const;
        ScriptFunction function_by_decl(const char* decl) const;
        ScriptFunction function_by_name(const char* name) const;
        ScriptFunction function_by_decl(const String& decl) const;
        ScriptFunction function_by_name(const String& name) const;

        // Global variables
        uint_t global_var_count() const;
        int_t global_var_index_by_name(const char* name) const;
        int_t global_var_index_by_decl(const char* decl) const;
        int_t global_var_index_by_name(const String& name) const;
        int_t global_var_index_by_decl(const String& decl) const;
        bool global_var(uint_t index, StringView* name = nullptr, StringView* name_space = nullptr, int_t* type_id = nullptr,
                        bool* is_const = nullptr) const;
        String global_var_declaration(uint_t index, bool include_namespace = false) const;
        void* address_of_global_var(uint_t index);

        // Type identification
        uint_t object_type_count() const;
        ScriptTypeInfo object_type_by_index(uint_t index) const;
        int_t type_id_by_decl(const char* decl) const;
        int_t type_id_by_decl(const String& decl) const;
        ScriptTypeInfo type_info_by_name(const char* name) const;
        ScriptTypeInfo type_info_by_decl(const char* decl) const;
        ScriptTypeInfo type_info_by_name(const String& name) const;
        ScriptTypeInfo type_info_by_decl(const String& decl) const;

        // Enums
        uint_t enum_count() const;
        ScriptTypeInfo enum_by_index(uint_t index) const;

        // Typedefs
        uint_t typedef_count() const;
        ScriptTypeInfo typedef_by_index(uint_t index) const;

        ~Script();
        friend class ScriptFolder;
    };
}// namespace Engine
