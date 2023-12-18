#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <ScriptEngine/script_function.hpp>



class asIScriptObject;

namespace Engine
{
    class ScriptTypeInfo;
    class ScriptFunction;

    class ENGINE_EXPORT ScriptObject
    {
    private:
        asIScriptObject* _M_object = nullptr;
        ScriptFunction _M_update;
        ScriptFunction _M_on_create;


        void bind_script_functions();

    public:
        ScriptObject(asIScriptObject* object = nullptr);
        copy_constructors_hpp(ScriptObject);

        ScriptObject& add_reference();
        ScriptObject& remove_reference();

        int_t type_id() const;
        ScriptTypeInfo object_type() const;
        bool is_valid() const;


        // Class properties
        uint_t property_count() const;
        int_t property_type_id(uint_t prop) const;
        const char* property_name(uint_t prop) const;
        void* get_address_of_property(uint_t prop);

        // Miscellaneous
        int_t copy_from(const ScriptObject& other);
        ~ScriptObject();


        // Class methods
        void update(float dt);
        void on_create(Object* owner);
        friend class ScriptFunction;
    };
}// namespace Engine
