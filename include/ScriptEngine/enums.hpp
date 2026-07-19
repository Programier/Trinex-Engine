#pragma once
#include <Core/engine_types.hpp>

namespace Trinex
{
	struct ScriptClassFlags {
		enum Enum : u64
		{
			Ref                      = 1ULL << 0,
			Value                    = 1ULL << 1,
			Pod                      = 1ULL << 3,
			Template                 = 1ULL << 6,
			AsHandle                 = 1ULL << 7,
			AppClass                 = 1ULL << 8,
			AppClassConstructor      = 1ULL << 9,
			AppClassDestructor       = 1ULL << 10,
			AppClassAssignment       = 1ULL << 11,
			AppClassCopyCtor         = 1ULL << 12,
			AppPrimitive             = 1ULL << 13,
			AppFloat                 = 1ULL << 14,
			AppArray                 = 1ULL << 15,
			AppClassAllInts          = 1ULL << 16,
			AppClassAllFloats        = 1ULL << 17,
			NoCount                  = 1ULL << 18,
			AppClassAlign8           = 1ULL << 19,
			ImplicitHandle           = 1ULL << 20,
			AppClassMoreConstructors = 1ULL << 31,
			AppNativeInheritance     = 1ULL << 33,
		};

		trinex_bitfield_enum_struct(ScriptClassFlags, u64);
	};

	struct ScriptModuleLookup {
		enum Enum : u8
		{
			OnlyIfExists      = 0,
			CreateIfNotExists = 1,
			AlwaysCreate      = 2,
		};

		trinex_enum_struct(ScriptModuleLookup);
	};

	struct ScriptTypeModifiers {
		enum Enum : u8
		{
			None     = 0,
			InRef    = 1,
			OutRef   = 2,
			InOutRef = 3,
			Const    = 4,
		};

		trinex_bitfield_enum_struct(ScriptTypeModifiers, u8);
	};

	struct ScriptCallConv {
		enum Enum
		{
			Auto              = 0,
			CDecl             = 1,
			StdCall           = 2,
			ThisCallAsGlobal  = 3,
			ThisCall          = 4,
			CDeclObjLast      = 5,
			CDeclObjFirst     = 6,
			Generic           = 7,
			ThisCall_ObjLast  = 8,
			ThisCall_ObjFirst = 9,
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
}// namespace Trinex
