#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    enum class ScriptCallConv : EnumerateType
    {
        CDECL             = 0,
        STDCALL           = 1,
        THISCALL_ASGLOBAL = 2,
        THISCALL          = 3,
        CDECL_OBJLAST     = 4,
        CDECL_OBJFIRST    = 5,
        GENERIC           = 6,
        THISCALL_OBJLAST  = 7,
        THISCALL_OBJFIRST = 8
    };


    enum class ScriptClassBehave : EnumerateType
    {
        Construct,
        ListConstruct,
        Destruct,

        Factory,
        ListFactory,
        AddRef,
        Release,
        GetWeakRefFlag,
        TemplateCallback,
        GetRefCount,
        GetGCFlag,
        SetGCFlag,
        EnumRefs,
        ReleaseRefs,
    };

    enum class ScriptTypeModifiers : EnumerateType
    {
        None     = 0,
        InRef    = 1,
        OutRef   = 2,
        InOutRef = 3,
        Const    = 4
    };
}// namespace Engine
