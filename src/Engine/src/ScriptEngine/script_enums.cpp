#include <Core/engine_loading_controllers.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_enums.hpp>

namespace Engine
{
    static void on_init()
    {
        {
            ScriptEnumRegistrar registrar("Engine::ScriptTypeModifiers");
            registrar.set("None", ScriptTypeModifiers::None);
            registrar.set("InRef", ScriptTypeModifiers::InRef);
            registrar.set("OutRef", ScriptTypeModifiers::OutRef);
            registrar.set("InOutRef", ScriptTypeModifiers::InOutRef);
            registrar.set("Const", ScriptTypeModifiers::Const);
        }

        {
            ScriptEnumRegistrar registrar("Engine::ScriptClassBehave");
            registrar.set("Construct", ScriptClassBehave::Construct);
            registrar.set("ListConstruct", ScriptClassBehave::ListConstruct);
            registrar.set("Destruct", ScriptClassBehave::Destruct);
            registrar.set("Factory", ScriptClassBehave::Factory);
            registrar.set("ListFactory", ScriptClassBehave::ListFactory);
            registrar.set("AddRef", ScriptClassBehave::AddRef);
            registrar.set("Release", ScriptClassBehave::Release);
            registrar.set("GetWeakRefFlag", ScriptClassBehave::GetWeakRefFlag);
            registrar.set("TemplateCallback", ScriptClassBehave::TemplateCallback);
            registrar.set("GetRefCount", ScriptClassBehave::GetRefCount);
            registrar.set("GetGCFlag", ScriptClassBehave::GetGCFlag);
            registrar.set("SetGCFlag", ScriptClassBehave::SetGCFlag);
            registrar.set("EnumRefs", ScriptClassBehave::EnumRefs);
            registrar.set("ReleaseRefs", ScriptClassBehave::ReleaseRefs);
        }
    }

    static ReflectionInitializeController initializer(on_init, "Engine::ScriptEnums");
}// namespace Engine
