#include <Core/engine_loading_controllers.hpp>
#include <ScriptEngine/script_engine.hpp>

#include <angelscript.h>

// Addons
#include <Core/string_functions.hpp>
#include <contextmgr.h>
#include <datetime.h>
#include <debugger.h>
#include <scriptany.h>
#include <scriptarray.h>
#include <scriptbuilder.h>
#include <scriptdictionary.h>
#include <scriptfile.h>
#include <scriptfilesystem.h>
#include <scriptgrid.h>
#include <scripthandle.h>
#include <scripthelper.h>
#include <scriptmath.h>
#include <scriptmathcomplex.h>
#include <scriptstdstring.h>
#include <weakref.h>


namespace Engine
{
    static String parse_string_value(const byte* address, int_t type_id)
    {
        return Strings::format("\"{}\"", *reinterpret_cast<const String*>(address));
    }


    static void on_init()
    {
        asIScriptEngine* engine = ScriptEngine::engine();

        RegisterScriptDateTime(engine);
        RegisterStdString(engine);
        RegisterScriptArray(engine, true);
        RegisterScriptAny(engine);
        RegisterScriptWeakRef(engine);
        RegisterScriptMathComplex(engine);
        RegisterScriptMath(engine);
        RegisterExceptionRoutines(engine);
        RegisterScriptHandle(engine);
        RegisterScriptGrid(engine);
        RegisterScriptFileSystem(engine);
        RegisterScriptFile(engine);
        RegisterScriptDictionary(engine);
        RegisterStdStringUtils(engine);

        int_t type_id = ScriptEngine::type_id_by_decl("string");
        ScriptEngine::register_custom_variable_parser(type_id, parse_string_value);
    }

    static ScriptAddonsInitializeController init(on_init, "Engine::DefaultAddons");
}// namespace Engine
