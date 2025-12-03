#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	struct ScriptModuleLookup {
		enum Enum : byte
		{
			OnlyIfExists      = 0,
			CreateIfNotExists = 1,
			AlwaysCreate      = 2,
		};

		trinex_enum_struct(ScriptModuleLookup);
	};

	struct ScriptTypeModifiers {
		enum Enum : byte
		{
			None     = 0,
			InRef    = 1,
			OutRef   = 2,
			InOutRef = 3,
			Const    = 4,
		};

		trinex_bitfield_enum_struct(ScriptTypeModifiers, byte);
	};

	struct ScriptCallConv {
		enum Enum
		{
			CDecl             = 0,
			StdCall           = 1,
			ThisCallAsGlobal  = 2,
			ThisCall          = 3,
			CDeclObjLast      = 4,
			CDeclObjFirst     = 5,
			Generic           = 6,
			ThisCall_ObjLast  = 7,
			ThisCall_ObjFirst = 8,
		};

		trinex_enum_struct(ScriptCallConv);
	};

	struct ScriptClassBehave {
		enum Enum : EnumerateType
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

		trinex_enum_struct(ScriptClassBehave);
	};
}// namespace Engine
