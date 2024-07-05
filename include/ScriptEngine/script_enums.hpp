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
        Construct        = 0,
        ListConstruct    = 1,
        Destruct         = 2,
        Factory          = 3,
        ListFactory      = 4,
        AddRef           = 5,
        Release          = 6,
        GetWeakRefFlag   = 7,
        TemplateCallback = 8,
        GetRefCount      = 9,
        GetGCFlag        = 10,
        SetGCFlag        = 11,
        EnumRefs         = 12,
        ReleaseRefs      = 13,
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
