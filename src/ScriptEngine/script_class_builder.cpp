#include <Core/class.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
    static ScriptClassRegistrar registrar_of(Class* self, bool exiting)
    {
        if (exiting)
        {
            return ScriptClassRegistrar::existing_class(self->name().to_string());
        }

        return ScriptClassRegistrar::reference_class(self->name().to_string());
    }

    void Class::bind_class_to_script_engine()
    {
        ScriptClassRegistrar registrar = registrar_of(this, true);

        Class* current = this;
        List<Class*> stack;

        while (current)
        {
            stack.push_back(current);
            current = current->parent();
        }

        while (!stack.empty())
        {
            current = stack.back();
            if (current->script_registration_callback)
            {
                current->script_registration_callback(&registrar, this);
            }

            stack.pop_back();
        }
    }

    void Class::register_scriptable_class()
    {
        ScriptClassRegistrar registrar = registrar_of(this, false);
        ScriptBindingsInitializeController().push([this]() { bind_class_to_script_engine(); });
    }
}// namespace Engine
