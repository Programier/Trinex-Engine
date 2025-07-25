#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	struct RHIShaderParameterType {
		enum Enum : EnumerateType
		{
			Undefined = 0,

			// Meta
			META_UniformBuffer       = BIT(16),
			META_Sampler             = BIT(17),
			META_Texture             = BIT(18),
			META_RWTexture           = BIT(19),
			META_Buffer              = BIT(20),
			META_RWBuffer            = BIT(21),
			META_StructuredBuffer    = BIT(22),
			META_RWStructuredBuffer  = BIT(23),
			META_ByteAddressBuffer   = BIT(24),
			META_RWByteAddressBuffer = BIT(25),

			META_AccelerationStructure = BIT(26),

			META_Scalar  = BIT(27),
			META_Vector  = BIT(28),
			META_Matrix  = BIT(29),
			META_Numeric = META_Scalar | META_Vector,

			META_ExcludeMaterialParameter = BIT(30),
			META_Any                      = 65535U << 16,

			// Scalars
			Bool          = 1 | META_Scalar | META_UniformBuffer,
			Bool2         = 2 | META_Vector | META_UniformBuffer,
			Bool3         = 3 | META_Vector | META_UniformBuffer,
			Bool4         = 4 | META_Vector | META_UniformBuffer,
			Int           = 5 | META_Scalar | META_UniformBuffer,
			Int2          = 6 | META_Vector | META_UniformBuffer,
			Int3          = 7 | META_Vector | META_UniformBuffer,
			Int4          = 8 | META_Vector | META_UniformBuffer,
			UInt          = 9 | META_Scalar | META_UniformBuffer,
			UInt2         = 10 | META_Vector | META_UniformBuffer,
			UInt3         = 11 | META_Vector | META_UniformBuffer,
			UInt4         = 12 | META_Vector | META_UniformBuffer,
			Float         = 13 | META_Scalar | META_UniformBuffer,
			Float2        = 14 | META_Vector | META_UniformBuffer,
			Float3        = 15 | META_Vector | META_UniformBuffer,
			Float4        = 16 | META_Vector | META_UniformBuffer,
			Float3x3      = 17 | META_Matrix | META_UniformBuffer,
			Float4x4      = 18 | META_Matrix | META_UniformBuffer,
			UniformBuffer = 19 | META_UniformBuffer,

			Sampler          = 20 | META_Sampler,
			Sampler1D        = 21 | META_Texture | META_Sampler,
			Sampler2D        = 22 | META_Texture | META_Sampler,
			Sampler3D        = 23 | META_Texture | META_Sampler,
			SamplerCube      = 24 | META_Texture | META_Sampler,
			Sampler1DArray   = 25 | META_Texture | META_Sampler,
			Sampler2DArray   = 26 | META_Texture | META_Sampler,
			SamplerCubeArray = 27 | META_Texture | META_Sampler,

			RWSampler1D        = 28 | META_RWTexture | META_Sampler,
			RWSampler2D        = 29 | META_RWTexture | META_Sampler,
			RWSampler3D        = 30 | META_RWTexture | META_Sampler,
			RWSamplerCube      = 31 | META_RWTexture | META_Sampler,
			RWSampler1DArray   = 32 | META_RWTexture | META_Sampler,
			RWSampler2DArray   = 33 | META_RWTexture | META_Sampler,
			RWSamplerCubeArray = 34 | META_RWTexture | META_Sampler,

			Texture1D        = 35 | META_Texture,
			Texture2D        = 36 | META_Texture,
			Texture3D        = 37 | META_Texture,
			TextureCube      = 38 | META_Texture,
			Texture1DArray   = 39 | META_Texture,
			Texture2DArray   = 40 | META_Texture,
			TextureCubeArray = 41 | META_Texture,

			RWTexture1D        = 42 | META_RWTexture,
			RWTexture2D        = 43 | META_RWTexture,
			RWTexture3D        = 44 | META_RWTexture,
			RWTextureCube      = 45 | META_RWTexture,
			RWTexture1DArray   = 46 | META_RWTexture,
			RWTexture2DArray   = 47 | META_RWTexture,
			RWTextureCubeArray = 48 | META_RWTexture,

			Buffer            = 49 | META_Buffer,
			StructuredBuffer  = 50 | META_StructuredBuffer,
			ByteAddressBuffer = 51 | META_ByteAddressBuffer,

			RWBuffer            = 52 | META_RWBuffer,
			RWStructuredBuffer  = 53 | META_RWStructuredBuffer,
			RWByteAddressBuffer = 54 | META_RWByteAddressBuffer,

			AccelerationStructure = 55 | META_AccelerationStructure,

			Globals         = 56 | META_UniformBuffer,
			LocalToWorld    = 57 | META_Matrix | META_UniformBuffer,
			Surface         = 58 | META_Texture,
			CombinedSurface = 59 | META_Texture | META_Sampler,
		};

		trinex_bitfield_enum_struct(RHIShaderParameterType, EnumerateType);
		trinex_declare_enum(RHIShaderParameterType);

		ENGINE_EXPORT RHIShaderParameterType make_vector(byte len);
		ENGINE_EXPORT RHIShaderParameterType make_scalar();

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

	struct RHITextureCreateFlags {
		enum Enum : byte
		{
			Undefined          = 0,
			ShaderResource     = BIT(0),
			UnorderedAccess    = BIT(1),
			RenderTarget       = BIT(2),
			DepthStencilTarget = BIT(3)
		};

		trinex_bitfield_enum_struct(RHITextureCreateFlags, byte);
	};

	struct RHIBufferCreateFlags {
		enum Enum : uint16_t
		{
			Undefined = 0,
			Static    = BIT(0),
			Dynamic   = BIT(1),

			VertexBuffer      = BIT(2),
			IndexBuffer       = BIT(3),
			UniformBuffer     = BIT(4),
			ShaderResource    = BIT(5),
			UnorderedAccess   = BIT(6),
			StructuredBuffer  = BIT(7),
			ByteAddressBuffer = BIT(8),
			TransferSrc       = BIT(9),
			TransferDst       = BIT(9),
			CPURead           = BIT(10),
			CPUWrite          = BIT(11),
		};

		trinex_bitfield_enum_struct(RHIBufferCreateFlags, uint16_t);
	};

	struct RHICompareFunc {
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

		trinex_enum_struct(RHICompareFunc);
		trinex_declare_enum(RHICompareFunc);
	};

	struct RHIAccess {
		enum Enum : EnumerateType
		{
			Undefined = 0,

			// Reading states
			CPURead       = BIT(0),
			Present       = BIT(1),
			IndirectArgs  = BIT(2),
			VertexBuffer  = BIT(3),
			IndexBuffer   = BIT(4),
			UniformBuffer = BIT(5),
			SRVCompute    = BIT(6),
			SRVGraphics   = BIT(7),
			CopySrc       = BIT(8),
			ResolveSrc    = BIT(9),

			// Writing states
			CPUWrite    = BIT(10),
			UAVCompute  = BIT(11),
			UAVGraphics = BIT(12),
			CopyDst     = BIT(13),
			ResolveDst  = BIT(14),
			RTV         = BIT(15),
			DSV         = BIT(16),

			ReadableMask = CPURead | Present | IndirectArgs | VertexBuffer | IndexBuffer | UniformBuffer | SRVCompute |
			               SRVGraphics | CopySrc | ResolveSrc | UAVCompute | UAVGraphics | RTV | DSV,

			WritableMask = CPUWrite | UAVCompute | UAVGraphics | CopyDst | ResolveDst | RTV | DSV,
		};

		trinex_bitfield_enum_struct(RHIAccess, EnumerateType);
	};

	struct RHIVertexBufferSemantic {
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

		trinex_enum_struct(RHIVertexBufferSemantic);
	};

	struct RHIVertexBufferElementType {
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

		trinex_enum_struct(RHIVertexBufferElementType);
	};

	struct RHISamplerFilter {
		enum Enum : EnumerateType
		{
			Point     = 0,
			Bilinear  = 1,
			Trilinear = 2,
		};

		trinex_enum_struct(RHISamplerFilter);
		trinex_declare_enum(RHISamplerFilter);
	};

	struct RHISamplerAddressMode {
		enum Enum : EnumerateType
		{
			Repeat            = 0,
			ClampToEdge       = 1,
			ClampToBorder     = 2,
			MirroredRepeat    = 3,
			MirrorClampToEdge = 4,
		};

		trinex_enum_struct(RHISamplerAddressMode);
		trinex_declare_enum(RHISamplerAddressMode);
	};

	struct RHIStencilOp {
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

		trinex_enum_struct(RHIStencilOp);
		trinex_declare_enum(RHIStencilOp);
	};

	struct RHIBlendFunc {
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

		trinex_enum_struct(RHIBlendFunc);
		trinex_declare_enum(RHIBlendFunc);
	};

	struct RHIBlendOp {
		enum Enum : EnumerateType
		{
			Add             = 0,
			Subtract        = 1,
			ReverseSubtract = 2,
			Min             = 3,
			Max             = 4,
		};

		trinex_enum_struct(RHIBlendOp);
		trinex_declare_enum(RHIBlendOp);
	};

	struct RHIPrimitive {
		enum Enum : EnumerateType
		{
			Triangle = 0,
			Line     = 1,
			Point    = 2,
		};

		trinex_enum_struct(RHIPrimitive);
		trinex_declare_enum(RHIPrimitive);
	};

	struct RHIPrimitiveTopology {
		enum Enum : EnumerateType
		{
			TriangleList  = 0,
			TriangleStrip = 1,
			LineList      = 2,
			LineStrip     = 3,
			PointList     = 4,
		};

		trinex_enum_struct(RHIPrimitiveTopology);
		trinex_declare_enum(RHIPrimitiveTopology);
	};

	struct RHIPolygonMode {
		enum Enum : EnumerateType
		{
			Fill  = 0,
			Line  = 1,
			Point = 2,
		};

		trinex_enum_struct(RHIPolygonMode);
		trinex_declare_enum(RHIPolygonMode);
	};

	struct RHICullMode {
		enum Enum : EnumerateType
		{
			None  = 0,
			Front = 1,
			Back  = 2,
		};

		trinex_enum_struct(RHICullMode);
		trinex_declare_enum(RHICullMode);
	};

	struct RHIFrontFace {
		enum Enum : EnumerateType
		{
			ClockWise        = 0,
			CounterClockWise = 1,
		};

		trinex_enum_struct(RHIFrontFace);
		trinex_declare_enum(RHIFrontFace);
	};

	struct RHIVertexAttributeInputRate {
		enum Enum : byte
		{
			Vertex   = 0,
			Instance = 1,
		};

		trinex_enum_struct(RHIVertexAttributeInputRate);
		trinex_declare_enum(RHIVertexAttributeInputRate);
	};

	struct RHIIndexFormat {
		enum Enum : byte
		{
			UInt32 = 0,
			UInt16 = 1,
		};

		trinex_enum_struct(RHIIndexFormat);
	};

	struct ENGINE_EXPORT RHIColorFormat {
		struct Capabilities {
			enum Enum : EnumerateType
			{
				Undefined = 0,

				Texture1D   = BIT(0),
				Texture2D   = BIT(1),
				Texture3D   = BIT(2),
				TextureCube = BIT(3),

				RenderTarget   = BIT(4),
				DepthStencil   = BIT(5),
				TextureMipmaps = BIT(6),

				TextureLoad      = BIT(7),
				TextureSample    = BIT(8),
				TextureGather    = BIT(9),
				TextureAtomics   = BIT(10),
				TextureBlendable = BIT(11),
				TextureStore     = BIT(12),
			};

			trinex_bitfield_enum_struct(Capabilities, EnumerateType);
		};

		enum Enum : EnumerateType
		{
			Undefined = 0,

			D32F      = 1,
			D16_UNORM = 3,
			D24S8     = 2,

			R8           = 4,
			R8G8         = 5,
			R8G8B8A8     = 7,
			R10G108B10A2 = 8,

			R8_SNORM       = 9,
			R8G8_SNORM     = 10,
			R8G8B8A8_SNORM = 12,

			R8_UINT       = 13,
			R8G8_UINT     = 14,
			R8G8B8A8_UINT = 16,

			R8_SINT       = 17,
			R8G8_SINT     = 18,
			R8G8B8A8_SINT = 20,

			R16          = 21,
			R16G16       = 22,
			R16G16B16A16 = 24,

			R16_SNORM          = 25,
			R16G16_SNORM       = 26,
			R16G16B16A16_SNORM = 28,

			R16_UINT          = 29,
			R16G16_UINT       = 30,
			R16G16B16A16_UINT = 32,

			R16_SINT          = 33,
			R16G16_SINT       = 34,
			R16G16B16A16_SINT = 36,

			R32_UINT          = 37,
			R32G32_UINT       = 38,
			R32G32B32A32_UINT = 40,

			R32_SINT          = 41,
			R32G32_SINT       = 42,
			R32G32B32A32_SINT = 44,

			R16F          = 45,
			R16G16F       = 46,
			R16G16B16A16F = 48,

			R32F          = 49,
			R32G32F       = 50,
			R32G32B32A32F = 52,

			BC1_RGBA = 53,
			BC2_RGBA = 54,
			BC3_RGBA = 55,
			BC4_R    = 56,
			BC5_RG   = 57,
			BC7_RGBA = 58,

			ASTC_4x4_RGBA   = 59,
			ASTC_6x6_RGBA   = 60,
			ASTC_8x8_RGBA   = 61,
			ASTC_10x10_RGBA = 62,

			ETC1_RGB  = 63,
			ETC2_RGB  = 64,
			ETC2_RGBA = 65,

			NV12 = 66,
			P010 = 67,
		};

		trinex_enum_struct(RHIColorFormat);
		trinex_declare_enum(RHIColorFormat);

		constexpr inline RHIColorFormat(const struct RHISurfaceFormat& format);
		constexpr inline RHIColorFormat& operator=(const struct RHISurfaceFormat& format);
		constexpr inline bool operator==(const struct RHISurfaceFormat& format) const noexcept;
		constexpr inline bool operator!=(const struct RHISurfaceFormat& format) const noexcept;

		Capabilities capabilities() const;
		RHIColorFormat add_capabilities(Capabilities capabilities) const;
		RHIColorFormat remove_capabilities(Capabilities capabilities) const;

		constexpr static inline uint32_t static_count() { return 68; }
		constexpr inline bool is_color() const { return value >= R8 && value <= P010; }
		constexpr inline bool is_depth_stencil() const { return value == D24S8; }
		constexpr inline bool is_depth() const { return value == D32F || value == D16_UNORM; }
		constexpr inline bool has_depth() const { return value >= D32F && value <= D24S8; }

		template<typename Func>
		constexpr static void static_foreach(Func&& func)
		{
			for (EnumerateType i = 1; i < static_count(); ++i)
			{
				func(RHIColorFormat(static_cast<Enum>(i)));
			}
		}
	};

	struct RHISurfaceFormat {
		enum Enum : EnumerateType
		{
			Undefined = RHIColorFormat::Undefined,
			D32F      = RHIColorFormat::D32F,
			D16       = RHIColorFormat::D16_UNORM,
			D24S8     = RHIColorFormat::D24S8,

			// Unsigned normalized formats,
			R8       = RHIColorFormat::R8,
			RG8      = RHIColorFormat::R8G8,
			RGBA8    = RHIColorFormat::R8G8B8A8,
			RG8B10A2 = RHIColorFormat::R10G108B10A2,
			R16      = RHIColorFormat::R16,
			RG16     = RHIColorFormat::R16G16,
			RGBA16   = RHIColorFormat::R16G16B16A16,

			// Signed normalized formats
			R8S     = RHIColorFormat::R8_SNORM,
			RG8S    = RHIColorFormat::R8G8_SNORM,
			RGBA8S  = RHIColorFormat::R8G8B8A8_SNORM,
			R16S    = RHIColorFormat::R16_SNORM,
			RG16S   = RHIColorFormat::R16G16_SNORM,
			RGBA16S = RHIColorFormat::R16G16B16A16_SNORM,

			// Unsigned integer formats
			R8UI     = RHIColorFormat::R8_UINT,
			RG8UI    = RHIColorFormat::R8G8_UINT,
			RGBA8UI  = RHIColorFormat::R8G8B8A8_UINT,
			R16UI    = RHIColorFormat::R16_UINT,
			RG16UI   = RHIColorFormat::R16G16_UINT,
			RGBA16UI = RHIColorFormat::R16G16B16A16_UINT,
			R32UI    = RHIColorFormat::R32_UINT,
			RG32UI   = RHIColorFormat::R32G32_UINT,
			RGBA32UI = RHIColorFormat::R32G32B32A32_UINT,

			// Signed integer formats
			R8SI     = RHIColorFormat::R8_SINT,
			RG8SI    = RHIColorFormat::R8G8_SINT,
			RGBA8SI  = RHIColorFormat::R8G8B8A8_SINT,
			R16SI    = RHIColorFormat::R16_SINT,
			RG16SI   = RHIColorFormat::R16G16_SINT,
			RGBA16SI = RHIColorFormat::R16G16B16A16_SINT,
			R32SI    = RHIColorFormat::R32_SINT,
			RG32SI   = RHIColorFormat::R32G32_SINT,
			RGBA32SI = RHIColorFormat::R32G32B32A32_SINT,

			// Floating formats
			R16F    = RHIColorFormat::R16F,
			RG16F   = RHIColorFormat::R16G16F,
			RGBA16F = RHIColorFormat::R16G16B16A16F,
			R32F    = RHIColorFormat::R32F,
			RG32F   = RHIColorFormat::R32G32F,
			RGBA32F = RHIColorFormat::R32G32B32A32F,
		};

		trinex_enum_struct(RHISurfaceFormat);
		trinex_declare_enum(RHISurfaceFormat);

		constexpr inline bool operator==(const struct RHIColorFormat& format) const noexcept
		{
			return value == static_cast<Enum>(format.value);
		}

		constexpr inline bool operator!=(const struct RHIColorFormat& format) const noexcept
		{
			return value != static_cast<Enum>(format.value);
		}

		inline RHIColorFormat as_color_format() const { return static_cast<RHIColorFormat::Enum>(value); }

		inline bool is_color() const { return as_color_format().is_color(); }
		inline bool is_depth_stencil() const { return as_color_format().is_depth_stencil(); }
		inline bool is_depth() const { return as_color_format().is_depth(); }
		inline bool has_depth() const { return as_color_format().has_depth(); }

		operator RHIColorFormat() const { return as_color_format(); }
	};

	constexpr inline RHIColorFormat::RHIColorFormat(const struct RHISurfaceFormat& format)
	    : value(static_cast<RHIColorFormat::Enum>(format.value))
	{}

	constexpr inline RHIColorFormat& RHIColorFormat::operator=(const struct RHISurfaceFormat& format)
	{
		value = static_cast<Enum>(format.value);
		return *this;
	}

	constexpr inline bool RHIColorFormat::operator==(const struct RHISurfaceFormat& format) const noexcept
	{
		return value == static_cast<Enum>(format.value);
	}

	constexpr inline bool RHIColorFormat::operator!=(const struct RHISurfaceFormat& format) const noexcept
	{
		return value != static_cast<Enum>(format.value);
	}

	struct RHIColorComponent {
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

		trinex_bitfield_enum_struct(RHIColorComponent, EnumerateType);
		trinex_declare_enum(RHIColorComponent);
	};

	struct RHIMappingAccess {
		enum Enum : byte
		{
			Read       = 1 << 0,
			Write      = 1 << 1,
			ReadWriter = 1 << 2,
		};

		trinex_enum_struct(RHIMappingAccess);
	};

	struct RHITextureType {
		enum Enum : byte
		{
			Undefined        = 0,
			Texture1D        = 1,
			Texture1DArray   = 2,
			Texture2D        = 3,
			Texture2DArray   = 4,
			TextureCube      = 5,
			TextureCubeArray = 6,
			Texture3D        = 7,
		};

		trinex_enum_struct(RHITextureType);
	};
};// namespace Engine
