#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <ScriptEngine/script_engine.hpp>


namespace Engine
{
    static void on_init()
    {
        static ScriptEngine* engine = ScriptEngine::instance();
        ScriptEngine::NamespaceSaverScoped save_namespace;
        engine->default_namespace("Engine");
#define s_typedef(a, b) engine->register_typedef(a, b)

        s_typedef("byte", "uint8");
        s_typedef("signed_byte", "int8");
        s_typedef("size_t", "uint64");
        s_typedef("Flags", "uint64");
        s_typedef("Point1D", "float");
        s_typedef("Offset1D", "float");
        s_typedef("Size1D", "float");
        s_typedef("Scale1D", "float");
        s_typedef("Translate1D", "float");
        s_typedef("EulerAngle1", "float");
        s_typedef("Distance", "float");


        s_typedef("ArrayIndex", "uint64");
        s_typedef("ArrayOffset", "uint64");
        s_typedef("PriorityIndex", "uint64");
        s_typedef("Counter", "uint64");
        s_typedef("Index", "uint64");
        s_typedef("MaterialLayoutIndex", "uint64");
    }


    static InitializeController initializer(on_init, "Bind Engine Types");
}// namespace Engine