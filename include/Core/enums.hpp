#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	enum class TextureType : EnumerateType
	{
		Texture2D,
		TextureCubeMap,
	};

	enum class CompareMode : EnumerateType
	{
		None,
		RefToTexture
	};

	enum class SamplerFilter : EnumerateType
	{
		Point     = 0,
		Bilinear  = 1,
		Trilinear = 2,
	};

	enum class Swizzle : EnumerateType
	{
		Identity = 0,
		Zero     = 1,
		One      = 2,
		R        = 3,
		G        = 4,
		B        = 5,
		A        = 6
	};

	enum class SamplerAddressMode : EnumerateType
	{
		Repeat            = 0,
		ClampToEdge       = 1,
		ClampToBorder     = 2,
		MirroredRepeat    = 3,
		MirrorClampToEdge = 4,
	};

	enum class TextureCubeMapFace : byte
	{
		Front = 0,
		Back  = 1,
		Up    = 2,
		Down  = 3,
		Left  = 4,
		Right = 5
	};

	enum class VertexBufferSemantic : EnumerateType
	{
		Position     = 0,
		TexCoord     = 1,
		Color        = 2,
		Normal       = 3,
		Tangent      = 4,
		Bitangent    = 5,
		BlendWeight  = 6,
		BlendIndices = 7,
	};

	enum class VertexBufferElementType : EnumerateType
	{
		Undefined = 0,
		Float1    = 1,
		Float2    = 2,
		Float3    = 3,
		Float4    = 4,

		Byte1 = 5,
		Byte2 = 6,
		Byte4 = 7,

		Byte1N = 8,
		Byte2N = 9,
		Byte4N = 10,

		UByte1 = 11,
		UByte2 = 12,
		UByte4 = 13,

		UByte1N = 14,
		UByte2N = 15,
		UByte4N = 16,
		Color   = 16,

		Short1 = 17,
		Short2 = 18,
		Short4 = 19,

		Short1N = 20,
		Short2N = 21,
		Short4N = 22,

		UShort1 = 23,
		UShort2 = 24,
		UShort4 = 25,

		UShort1N = 26,
		UShort2N = 27,
		UShort4N = 28,

		Int1 = 29,
		Int2 = 30,
		Int3 = 31,
		Int4 = 32,

		UInt1 = 33,
		UInt2 = 34,
		UInt3 = 35,
		UInt4 = 36
	};

	enum class Coord
	{
		X,
		Y,
		Z
	};

	enum class DataType : EnumerateType
	{
		Text = 0,
		Binary
	};


	enum class OperationSystemType
	{
		Linux,
		Windows,
		Android
	};

	enum class ColorComponent : EnumerateType
	{
		R = 1,
		G = 2,
		B = 4,
		A = 8,
	};

	declare_enum_operators(ColorComponent);

	enum class CompareFunc : EnumerateType
	{
		Always   = 0,
		Lequal   = 1,
		Gequal   = 2,
		Less     = 3,
		Greater  = 4,
		Equal    = 5,
		NotEqual = 6,
		Never    = 7,
	};

	enum class PhysicalSizeMetric
	{
		Inch,
		Сentimeters,
	};

	enum class AccessType
	{
		Private,
		Protected,
		Public
	};

	enum class StencilOp : EnumerateType
	{
		Keep     = 0,
		Zero     = 1,
		Replace  = 2,
		Incr     = 3,
		IncrWrap = 4,
		Decr     = 5,
		DecrWrap = 6,
		Invert   = 7,
	};

	enum class BlendFunc : EnumerateType
	{
		Zero                = 0,
		One                 = 1,
		SrcColor            = 2,
		OneMinusSrcColor    = 3,
		DstColor            = 4,
		OneMinusDstColor    = 5,
		SrcAlpha            = 6,
		OneMinusSrcAlpha    = 7,
		DstAlpha            = 8,
		OneMinusDstAlpha    = 9,
		BlendFactor         = 10,
		OneMinusBlendFactor = 11,
	};

	enum class BlendOp : EnumerateType
	{
		Add             = 0,
		Subtract        = 1,
		ReverseSubtract = 2,
		Min             = 3,
		Max             = 4,
	};

	enum class Primitive : EnumerateType
	{
		Triangle = 0,
		Line     = 1,
		Point    = 2,
	};

	enum class PrimitiveTopology : EnumerateType
	{
		TriangleList  = 0,
		TriangleStrip = 1,
		LineList      = 2,
		LineStrip     = 3,
		PointList     = 4,
	};

	enum class PolygonMode : EnumerateType
	{
		Fill  = 0,
		Line  = 1,
		Point = 2,
	};

	enum class CullMode : EnumerateType
	{
		None  = 0,
		Front = 1,
		Back  = 2,
	};

	enum class FrontFace : EnumerateType
	{
		ClockWise        = 0,
		CounterClockWise = 1,
	};

	enum class WindowAttribute : EnumerateType
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

	enum class CursorMode : EnumerateType
	{
		Normal,
		Hidden
	};

	enum class Orientation : EnumerateType
	{
		Landscape        = 0,
		LandscapeFlipped = 1,
		Portrait         = 2,
		PortraitFlipped  = 3
	};

	enum class MessageBoxType
	{
		Error,
		Warning,
		Info,
	};

	enum class VertexAttributeInputRate : byte
	{
		Vertex   = 0,
		Instance = 1
	};

	enum class ColorComponentMask : EnumerateType
	{
		RGBA = static_cast<EnumerateType>(ColorComponent::R | ColorComponent::G | ColorComponent::B | ColorComponent::A),
		RGB  = static_cast<EnumerateType>(ColorComponent::R | ColorComponent::G | ColorComponent::B),
		RGA  = static_cast<EnumerateType>(ColorComponent::R | ColorComponent::G | ColorComponent::A),
		RG   = static_cast<EnumerateType>(ColorComponent::R | ColorComponent::G),
		RBA  = static_cast<EnumerateType>(ColorComponent::R | ColorComponent::B | ColorComponent::A),
		RB   = static_cast<EnumerateType>(ColorComponent::R | ColorComponent::B),
		RA   = static_cast<EnumerateType>(ColorComponent::R | ColorComponent::A),
		R    = static_cast<EnumerateType>(ColorComponent::R),
		GBA  = static_cast<EnumerateType>(ColorComponent::G | ColorComponent::B | ColorComponent::A),
		GB   = static_cast<EnumerateType>(ColorComponent::G | ColorComponent::B),
		GA   = static_cast<EnumerateType>(ColorComponent::G | ColorComponent::A),
		G    = static_cast<EnumerateType>(ColorComponent::G),
		BA   = static_cast<EnumerateType>(ColorComponent::B | ColorComponent::A),
		B    = static_cast<EnumerateType>(ColorComponent::B),
		A    = static_cast<EnumerateType>(ColorComponent::A)
	};

	enum class SerializationFlags : EnumerateType
	{
		None             = 0,
		SkipObjectSearch = BIT(0),

		IsCopyProcess = BIT(1),
	};

	enum class BufferSeekDir : EnumerateType
	{
		Current = 0,
		Begin   = 1,
		End     = 2,
	};

	enum class FileOpenMode : EnumerateType
	{
		In        = 0x01,
		Out       = 0x02,
		ReadWrite = In | Out,
		Append    = 0x04,
		Trunc     = 0x08
	};
	using FileSeekDir = BufferSeekDir;

	enum class RenderPassType : EnumerateType
	{
		Undefined  = 0,
		Window     = 1,
		SceneColor = 2,
		GBuffer    = 3,
		Depth      = 4,
		__COUNT__  = 4,
	};

	enum class ViewMode : EnumerateType
	{
		Lit         = 0,
		Unlit       = 1,
		Wireframe   = 2,
		WorldNormal = 3,
		Metalic     = 4,
		Roughness   = 5,
		Specular    = 6,
		AO          = 7,
	};

	enum class ShaderType : EnumerateType
	{
		Undefined           = 0,
		Vertex              = BIT(0),
		TessellationControl = BIT(1),
		Tessellation        = BIT(2),
		Geometry            = BIT(3),
		Fragment            = BIT(4),
		Compute             = BIT(5),
	};

	declare_enum_operators(ShaderType);

	enum class RHIBufferType : EnumerateType
	{
		Static  = 0,
		Dynamic = 1,
	};

	enum class ColorFormat : EnumerateType
	{
		Undefined    = 0,
		FloatR       = 1,
		FloatRGBA    = 2,
		R8           = 3,
		R8G8B8A8     = 4,
		Depth        = 5,
		DepthStencil = 6,
		ShadowDepth  = 7,
		BC1          = 8,
		BC2          = 9,
		BC3          = 10,
	};

	enum class ShowFlags : EnumerateType
	{
		None              = 0,
		Sprite            = BIT(0),
		Wireframe         = BIT(1),
		Gizmo             = BIT(2),
		PointLights       = BIT(3),
		SpotLights        = BIT(4),
		DirectionalLights = BIT(5),
		PostProcess       = BIT(6),

		StaticMesh = BIT(7),

		LightOctree     = BIT(8),
		PrimitiveOctree = BIT(9),
		Statistics      = BIT(10),

		DefaultFlags = Sprite | PointLights | SpotLights | DirectionalLights | StaticMesh | PostProcess
	};

	declare_enum_operators(ShowFlags);

	enum class MaterialDomain : EnumerateType
	{
		Surface = 0,
	};

	enum class SplashTextType : EnumerateType
	{
		StartupProgress = 0,
		VersionInfo     = 1,
		CopyrightInfo   = 2,
		GameName        = 3,
		Count           = 4
	};

	enum class IndexBufferFormat
	{
		UInt32 = 0,
		UInt16 = 1,
	};

	enum class ScriptCallConv
	{
		CDecl             = 0,
		StdCall           = 1,
		ThisCallAsGlobal  = 2,
		ThisCall          = 3,
		CDeclObjLast      = 4,
		CDeclObjFirst     = 5,
		Generic           = 6,
		ThisCall_ObjLast  = 7,
		ThisCall_ObjFirst = 8
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
