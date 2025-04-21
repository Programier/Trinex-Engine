#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	struct TextureType {
		enum Enum : EnumerateType
		{
			Texture2D,
			TextureCubeMap,
		};

		trinex_enum_struct(TextureType);
		trinex_declare_enum(TextureType);
	};

	struct CompareMode {
		enum Enum : EnumerateType
		{
			None,
			RefToTexture
		};

		trinex_enum_struct(CompareMode);
		trinex_declare_enum(CompareMode);
	};

	struct SamplerFilter {
		enum Enum : EnumerateType
		{
			Point     = 0,
			Bilinear  = 1,
			Trilinear = 2,
		};

		trinex_enum_struct(SamplerFilter);
		trinex_declare_enum(SamplerFilter);
	};

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

	struct SamplerAddressMode {
		enum Enum : EnumerateType
		{
			Repeat            = 0,
			ClampToEdge       = 1,
			ClampToBorder     = 2,
			MirroredRepeat    = 3,
			MirrorClampToEdge = 4,
		};

		trinex_enum_struct(SamplerAddressMode);
		trinex_declare_enum(SamplerAddressMode);
	};

	struct TextureCubeMapFace {
		enum Enum : byte
		{
			Front = 0,
			Back  = 1,
			Up    = 2,
			Down  = 3,
			Left  = 4,
			Right = 5,
		};

		trinex_enum_struct(TextureCubeMapFace);
		trinex_declare_enum(TextureCubeMapFace);
	};

	struct VertexBufferSemantic {
		enum Enum : EnumerateType
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

		trinex_enum_struct(VertexBufferSemantic);
		trinex_declare_enum(VertexBufferSemantic);
	};

	struct VertexBufferElementType {
		enum Enum : EnumerateType
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
			UInt4 = 36,
		};

		trinex_enum_struct(VertexBufferElementType);
		trinex_declare_enum(VertexBufferElementType);
	};

	struct Coord {
		enum Enum
		{
			X,
			Y,
			Z,
		};

		trinex_enum_struct(Coord);
		trinex_declare_enum(Coord);
	};

	struct DataType {
		enum Enum : EnumerateType
		{
			Text = 0,
			Binary,
		};

		trinex_enum_struct(DataType);
		trinex_declare_enum(DataType);
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

	struct ColorComponent {
		enum Enum : EnumerateType
		{
			R = 1,
			G = 2,
			B = 4,
			A = 8,

			RG   = R | G,
			RGB  = R | G | B,
			RGBA = R | G | B | A,
		};

		trinex_bitfield_enum_struct(ColorComponent, EnumerateType);
		trinex_declare_enum(ColorComponent);
	};

	struct CompareFunc {
		enum Enum : EnumerateType
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

		trinex_enum_struct(CompareFunc);
		trinex_declare_enum(CompareFunc);
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

	struct AccessType {
		enum Enum
		{
			Private,
			Protected,
			Public,
		};

		trinex_enum_struct(AccessType);
		trinex_declare_enum(AccessType);
	};

	struct StencilOp {
		enum Enum : EnumerateType
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

		trinex_enum_struct(StencilOp);
		trinex_declare_enum(StencilOp);
	};

	struct BlendFunc {
		enum Enum : EnumerateType
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

		trinex_enum_struct(BlendFunc);
		trinex_declare_enum(BlendFunc);
	};

	struct BlendOp {
		enum Enum : EnumerateType
		{
			Add             = 0,
			Subtract        = 1,
			ReverseSubtract = 2,
			Min             = 3,
			Max             = 4,
		};

		trinex_enum_struct(BlendOp);
		trinex_declare_enum(BlendOp);
	};

	struct Primitive {
		enum Enum : EnumerateType
		{
			Triangle = 0,
			Line     = 1,
			Point    = 2,
		};

		trinex_enum_struct(Primitive);
		trinex_declare_enum(Primitive);
	};

	struct PrimitiveTopology {
		enum Enum : EnumerateType
		{
			TriangleList  = 0,
			TriangleStrip = 1,
			LineList      = 2,
			LineStrip     = 3,
			PointList     = 4,
		};

		trinex_enum_struct(PrimitiveTopology);
		trinex_declare_enum(PrimitiveTopology);
	};

	struct PolygonMode {
		enum Enum : EnumerateType
		{
			Fill  = 0,
			Line  = 1,
			Point = 2,
		};

		trinex_enum_struct(PolygonMode);
		trinex_declare_enum(PolygonMode);
	};

	struct CullMode {
		enum Enum : EnumerateType
		{
			None  = 0,
			Front = 1,
			Back  = 2,
		};

		trinex_enum_struct(CullMode);
		trinex_declare_enum(CullMode);
	};

	struct FrontFace {
		enum Enum : EnumerateType
		{
			ClockWise        = 0,
			CounterClockWise = 1,
		};

		trinex_enum_struct(FrontFace);
		trinex_declare_enum(FrontFace);
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

	struct VertexAttributeInputRate {
		enum Enum : byte
		{
			Vertex   = 0,
			Instance = 1,
		};

		trinex_enum_struct(VertexAttributeInputRate);
		trinex_declare_enum(VertexAttributeInputRate);
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
			In        = 0x01,
			Out       = 0x02,
			ReadWrite = In | Out,
			Append    = 0x04,
			Trunc     = 0x08,
		};

		trinex_bitfield_enum_struct(FileOpenMode, byte);
	};

	using FileSeekDir = BufferSeekDir;

	struct RenderPassType {
		enum Enum : EnumerateType
		{
			Undefined  = 0,
			Window     = 1,
			SceneColor = 2,
			GBuffer    = 3,
			Depth      = 4,
			__COUNT__  = 4,
		};

		trinex_enum_struct(RenderPassType);
		trinex_declare_enum(RenderPassType);
	};

	struct ViewMode {
		enum Enum : EnumerateType
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

		trinex_enum_struct(ViewMode);
		trinex_declare_enum(ViewMode);
	};

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

	struct RHIBufferType {
		enum Enum : byte
		{
			Static  = 0,
			Dynamic = 1,
		};

		trinex_enum_struct(RHIBufferType);
	};

	struct ColorFormat {
		enum Enum : EnumerateType
		{
			Undefined         = 0,
			META_Color        = BIT(0),
			META_Depth        = BIT(1),
			META_DepthStencil = BIT(2) | META_Depth,

			FloatR       = (1 << 3) | META_Color,
			FloatRGBA    = (2 << 3) | META_Color,
			R8           = (3 << 3) | META_Color,
			R8G8B8A8     = (4 << 3) | META_Color,
			Depth        = (5 << 3) | META_Depth,
			DepthStencil = (6 << 3) | META_DepthStencil,
			ShadowDepth  = (7 << 3) | META_Depth,
			BC1          = (8 << 3) | META_Color,
			BC2          = (9 << 3) | META_Color,
			BC3          = (10 << 3) | META_Color,
		};

		trinex_enum_struct(ColorFormat);
		trinex_declare_enum(ColorFormat);

		inline bool is_color() const { return static_cast<EnumerateType>(value) & META_Color; }
		inline bool is_depth() const { return static_cast<EnumerateType>(value) & META_Depth; }
		inline bool is_depth_stencil() const
		{
			return (static_cast<EnumerateType>(value) & META_DepthStencil) == META_DepthStencil;
		}
	};

	struct ShowFlags {
		enum Enum : EnumerateType
		{
			None              = 0,
			Sprite            = BIT(0),
			Wireframe         = BIT(1),
			Gizmo             = BIT(2),
			PointLights       = BIT(3),
			SpotLights        = BIT(4),
			DirectionalLights = BIT(5),
			PostProcess       = BIT(6),
			StaticMesh        = BIT(7),
			LightOctree       = BIT(8),
			PrimitiveOctree   = BIT(9),
			Statistics        = BIT(10),

			DefaultFlags = Sprite | PointLights | SpotLights | DirectionalLights | StaticMesh | PostProcess,
		};

		trinex_bitfield_enum_struct(ShowFlags, EnumerateType);
		trinex_declare_enum(ShowFlags);
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

	struct RHIIndexFormat {
		enum Enum : byte
		{
			UInt32 = 0,
			UInt16 = 1,
		};

		trinex_enum_struct(RHIIndexFormat);
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

	struct ShaderParameterType {
		enum Enum : EnumerateType
		{
			Undefined = 0,

			// Meta
			META_UniformBuffer = BIT(16),
			META_Sampler       = BIT(17),
			META_Texture       = BIT(18),
			META_RWTexture     = BIT(19),
			META_TexelBuffer   = BIT(20),
			META_RWTexelBuffer = BIT(21),
			META_Buffer        = BIT(22),
			META_RWBuffer      = BIT(23),

			META_Scalar  = BIT(24),
			META_Vector  = BIT(25),
			META_Matrix  = BIT(26),
			META_Numeric = META_Scalar | META_Vector,
			META_Any     = 65535U << 16,

			// Values
			Bool        = 1 | META_Scalar | META_UniformBuffer,
			Bool2       = 2 | META_Vector | META_UniformBuffer,
			Bool3       = 3 | META_Vector | META_UniformBuffer,
			Bool4       = 4 | META_Vector | META_UniformBuffer,
			Int         = 5 | META_Scalar | META_UniformBuffer,
			Int2        = 6 | META_Vector | META_UniformBuffer,
			Int3        = 7 | META_Vector | META_UniformBuffer,
			Int4        = 8 | META_Vector | META_UniformBuffer,
			UInt        = 9 | META_Scalar | META_UniformBuffer,
			UInt2       = 10 | META_Vector | META_UniformBuffer,
			UInt3       = 11 | META_Vector | META_UniformBuffer,
			UInt4       = 12 | META_Vector | META_UniformBuffer,
			Float       = 13 | META_Scalar | META_UniformBuffer,
			Float2      = 14 | META_Vector | META_UniformBuffer,
			Float3      = 15 | META_Vector | META_UniformBuffer,
			Float4      = 16 | META_Vector | META_UniformBuffer,
			Float3x3    = 17 | META_Matrix | META_UniformBuffer,
			Float4x4    = 18 | META_Matrix | META_UniformBuffer,
			MemoryBlock = 19 | META_UniformBuffer,
			Sampler     = 20 | META_Sampler,
			Sampler2D   = 21 | META_Texture | META_Sampler,
			Texture2D   = 22 | META_Texture,

			// RW Resources
			RWTexture2D = 23 | META_RWTexture,

			Globals         = 24 | META_UniformBuffer,
			LocalToWorld    = 25 | META_Matrix | META_UniformBuffer,
			Surface         = 26 | META_Texture,
			CombinedSurface = 27 | META_Texture | META_Sampler,
		};

		trinex_bitfield_enum_struct(ShaderParameterType, EnumerateType);
		trinex_declare_enum(ShaderParameterType);

		ShaderParameterType make_vector(byte len);
		ShaderParameterType make_scalar();

		inline constexpr bool is_scalar() const { return (value & META_Scalar) == META_Scalar; }
		inline constexpr bool is_vector() const { return (value & META_Vector) == META_Vector; }
		inline constexpr bool is_matrix() const { return (value & META_Matrix) == META_Matrix; }
		inline constexpr bool is_numeric() const { return (value & META_Numeric) != 0; }
		inline constexpr bool is_meta() const { return (value & 0xFFFF) == 0; }
		inline constexpr uint16_t type_index() const { return (value & 0xFFFF) - 1; }

		inline constexpr byte vector_length() const
		{
			if (!is_vector() && !is_scalar())
				return 0;
			return (((value & 0xFFFF) - 1) % 4) + 1;
		}
	};

	struct TextureCreateFlags {
		enum Enum
		{
			None               = 0,
			ShaderResource     = BIT(0),
			UnorderedAccess    = BIT(1),
			RenderTarget       = BIT(2),
			DepthStencilTarget = BIT(3),
		};

		trinex_bitfield_enum_struct(TextureCreateFlags, byte);
	};
}// namespace Engine
