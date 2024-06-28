#include <Core/enum.hpp>
#include <ScriptEngine/script_enums.hpp>

namespace Engine
{
    implement_enum(ScriptTypeModifiers, Engine, {"None", ScriptTypeModifiers::None}, {"InRef", ScriptTypeModifiers::InRef},
                   {"OutRef", ScriptTypeModifiers::OutRef}, {"InOutRef", ScriptTypeModifiers::InOutRef},
                   {"Const", ScriptTypeModifiers::Const});
}// namespace Engine
