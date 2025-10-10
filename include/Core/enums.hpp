#pragma once
#include <Core/engine_types.hpp>
#include <Core/math/vector.hpp>

namespace Engine
{
	struct Swizzle : VectorNT<4, byte> {
		enum Enum : byte
		{
			R    = 0,
			G    = 1,
			B    = 2,
			A    = 3,
			Zero = 4,
			One  = 5,
		};

		inline Swizzle() : VectorNT<4, byte>(R, G, B, A) {}
		using VectorNT<4, byte>::VectorNT;
	};

	struct OperationSystemType {
		enum Enum
		{
			Linux,
			Windows,
			Android,
		};

		trinex_enum_struct(OperationSystemType);
		trinex_declare_enum(OperationSystemType);
	};

	struct PhysicalSizeMetric {
		enum Enum
		{
			Inch,
			Сentimeters,
		};

		trinex_enum_struct(PhysicalSizeMetric);
		trinex_declare_enum(PhysicalSizeMetric);
	};

	struct WindowAttribute {
		enum Enum : EnumerateType
		{
			None            = 0,
			Resizable       = 1,
			FullScreen      = 2,
			Shown           = 3,
			Hidden          = 4,
			BorderLess      = 5,
			MouseFocus      = 6,
			InputFocus      = 7,
			InputGrabbed    = 8,
			Minimized       = 9,
			Maximized       = 10,
			MouseCapture    = 12,
			MouseGrabbed    = 14,
			KeyboardGrabbed = 15,
		};

		trinex_enum_struct(WindowAttribute);
		trinex_declare_enum(WindowAttribute);
	};

	struct CursorMode {
		enum Enum : EnumerateType
		{
			Normal,
			Hidden,
		};

		trinex_enum_struct(CursorMode);
		trinex_declare_enum(CursorMode);
	};

	struct Orientation {
		enum Enum : EnumerateType
		{
			Landscape        = 0,
			LandscapeFlipped = 1,
			Portrait         = 2,
			PortraitFlipped  = 3,
		};

		trinex_enum_struct(Orientation);
		trinex_declare_enum(Orientation);
	};

	struct MessageBoxType {
		enum Enum
		{
			Error,
			Warning,
			Info,
		};

		trinex_enum_struct(MessageBoxType);
		trinex_declare_enum(MessageBoxType);
	};

	struct SerializationFlags {
		enum Enum : EnumerateType
		{
			None             = 0,
			SkipObjectSearch = BIT(0),
			IsCopyProcess    = BIT(1),
		};

		trinex_bitfield_enum_struct(SerializationFlags, EnumerateType);
	};

	struct BufferSeekDir {
		enum Enum : EnumerateType
		{
			Current = 0,
			Begin   = 1,
			End     = 2,
		};

		trinex_enum_struct(BufferSeekDir);
	};

	struct FileOpenMode {
		enum Enum : byte
		{
			In        = BIT(0),
			Out       = BIT(1),
			ReadWrite = In | Out,
			Append    = BIT(2) | Out,
		};

		trinex_bitfield_enum_struct(FileOpenMode, byte);
	};

	using FileSeekDir = BufferSeekDir;

	struct ShaderType {
		enum Enum : EnumerateType
		{
			Undefined           = 0,
			Vertex              = BIT(0),
			TessellationControl = BIT(1),
			Tessellation        = BIT(2),
			Geometry            = BIT(3),
			Fragment            = BIT(4),
			Compute             = BIT(5),

			All           = Vertex | TessellationControl | Tessellation | Geometry | Fragment | Compute,
			BasicGraphics = Vertex | Fragment,
		};

		trinex_bitfield_enum_struct(ShaderType, EnumerateType);
	};

	struct MaterialDomain {
		enum Enum : EnumerateType
		{
			Surface     = 0,
			PostProcess = 1,
		};

		trinex_enum_struct(MaterialDomain);
		trinex_declare_enum(MaterialDomain);
	};

	struct SplashTextType {
		enum Enum : EnumerateType
		{
			StartupProgress = 0,
			VersionInfo     = 1,
			CopyrightInfo   = 2,
			GameName        = 3,
			Count           = 4,
		};

		trinex_enum_struct(SplashTextType);
		trinex_declare_enum(SplashTextType);
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
		trinex_declare_enum(ScriptCallConv);
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
		trinex_declare_enum(ScriptClassBehave);
	};

	struct ScriptTypeModifiers {
		enum Enum : EnumerateType
		{
			None     = 0,
			InRef    = 1,
			OutRef   = 2,
			InOutRef = 3,
			Const    = 4,
		};

		trinex_enum_struct(ScriptTypeModifiers);
	};
}// namespace Engine
