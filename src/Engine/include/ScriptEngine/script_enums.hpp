#pragma once

namespace Engine
{
    enum class ScriptCallConv : unsigned int
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


    enum class ScriptClassBehave
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
}
