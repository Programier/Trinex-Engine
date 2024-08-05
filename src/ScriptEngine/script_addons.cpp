#include <Core/engine_loading_controllers.hpp>
#include <ScriptEngine/script_engine.hpp>

#include <angelscript.h>

// Addons
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

static Engine::ScriptAddonsInitializeController on_init(
        []() {
            asIScriptEngine* engine = Engine::ScriptEngine::engine();

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
        },
        "Engine::DefaultAddons");
