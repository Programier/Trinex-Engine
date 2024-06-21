#pragma once
#include <Core/engine_types.hpp>
#include <ScriptEngine/script_enums.hpp>

class asIScriptEngine;
class asIScriptContext;
class asIJITCompiler;

namespace Engine
{
    class ScriptModule;
    class ScriptTypeInfo;
    class ScriptObject;
    class ScriptFunction;


    class ENGINE_EXPORT ScriptEngine
    {
    private:
        Vector<class Script*> m_scripts;
        static ScriptEngine* m_instance;
        asIScriptEngine* m_engine;
        asIJITCompiler* m_jit_compiler = nullptr;

        ScriptEngine();
        ~ScriptEngine();

        void release_scripts();

        static void initialize();
        static void terminate();
        asIScriptContext* new_context() const;
        ScriptEngine& private_register_function(const char* declaration, void* func, ScriptCallConv conv);
        ScriptEngine& destroy_script_object(ScriptObjectAddress, const ScriptTypeInfo& info);

    public:
        class ENGINE_EXPORT NamespaceSaverScoped final
        {
            String m_ns;

        public:
            NamespaceSaverScoped();
            ~NamespaceSaverScoped();
        };

        enum GarbageCollectFlags
        {
            FullCycle      = 1,
            OneStep        = 2,
            DestroyGarbage = 4,
            DetectGarbage  = 8
        };

        static ScriptEngine* instance();
        asIScriptEngine* as_engine() const;
        const ScriptEngine& release_context(asIScriptContext* context) const;

        ScriptEngine& default_namespace(const String& name);
        ScriptEngine& default_namespace(const char* ns);
        String default_namespace() const;
        int_t register_property(const char* declaration, void* data);
        int_t register_property(const String& declaration, void* data);
        ScriptModule global_module() const;
        ScriptModule create_module(const String& name, EnumerateType flags = 0) const;
        ScriptModule create_module(const char* name, EnumerateType flags = 0) const;
        ScriptModule module(uint_t index);
        uint_t module_count() const;
        String to_string(const void* object, int_t type_id) const;

        class Script* new_script(const Path& path);
        const Vector<class Script*>& scripts() const;
        ScriptEngine& load_scripts();

        ScriptEngine& bind_imports();
        ScriptEngine& funcdef(const char* declaration);
        ScriptEngine& funcdef(const String& declaration);
        ScriptEngine& register_typedef(const char* new_type_name, const char* type);
        ScriptEngine& register_typedef(const String& new_type_name, const String& type);
        ScriptObject create_script_object(const ScriptTypeInfo& info, bool uninited = false) const;

        uint_t global_function_count() const;
        ScriptFunction global_function_by_index(uint_t index) const;
        ScriptFunction global_function_by_decl(const char* declaration) const;
        ScriptFunction global_function_by_decl(const String& declaration) const;

        ScriptEngine& garbage_collect(BitMask flags = GarbageCollectFlags::FullCycle, size_t iterations = 1);

        uint_t object_type_count() const;
        ScriptTypeInfo object_type_by_index(uint_t index) const;

        bool exec_string(const String& line) const;
        bool exec_string(const char* line) const;

        // Enums
        uint_t enum_count() const;
        ScriptTypeInfo enum_by_index(uint_t index) const;

        // Funcdefs
        uint_t funcdef_count() const;
        ScriptTypeInfo funcdef_by_index(uint_t index) const;

        // Typedefs
        uint_t typedef_count() const;
        ScriptTypeInfo typedef_by_index(uint_t index) const;

        // Script modules
        ScriptEngine& discard_module(const char* module);
        ScriptEngine& discard_module(const String& module);
        ScriptModule module_by_index(uint_t index) const;

        // Script functions
        int_t last_function_id() const;
        ScriptFunction function_by_id(int func_id) const;

        // Type identification
        int_t typeid_by_decl(const char* decl) const;
        const char* type_declaration(int_t type_id, bool include_namespace = false) const;
        int_t sizeof_primitive_type(int_t type_id) const;
        ScriptTypeInfo type_info_by_id(int_t type_id) const;
        ScriptTypeInfo type_info_by_name(const char* name) const;
        ScriptTypeInfo type_info_by_decl(const char* decl) const;
        ScriptTypeInfo type_info_by_name(const String& name) const;
        ScriptTypeInfo type_info_by_decl(const String& decl) const;


        template<typename T>
        ScriptEngine& register_function(const char* declaration, T func, ScriptCallConv conv = ScriptCallConv::CDECL)
        {
            return private_register_function(declaration, reinterpret_cast<void*>(func), conv);
        }

        template<typename T>
        ScriptEngine& register_function(const String& declaration, T func)
        {
            return register_function(declaration.c_str(), func);
        }

        friend class ScriptFunction;
        friend class ScriptClassRegistrar;
        friend class ScriptObject;
        friend class EngineLoop;
    };
}// namespace Engine
