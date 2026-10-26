#pragma once
#include <RHI/types.hpp>

namespace Trinex
{
	struct RHIBackend
	{
		enum Enum : u8
		{
			Undefined = 0,
			Vulkan    = 1,
			DirectX11 = 2,
			DirectX12 = 3,
			Metal     = 4,
			OpenGL    = 5,
			OpenGLES  = 6,
			WebGPU    = 7,
		};

		trinex_enum_struct(RHIBackend);
	};

	struct RHIShaderParameterType
	{
		enum Enum : EnumerateType
		{
			Undefined = 0,

			// Meta
			META_Descriptor            = trinex_bit(0),
			META_UniformBuffer         = trinex_bit(1),
			META_Sampler               = trinex_bit(2),
			META_Texture               = trinex_bit(3),
			META_Buffer                = trinex_bit(4),
			META_StructuredBuffer      = trinex_bit(5),
			META_ByteAddressBuffer     = trinex_bit(6),
			META_AccelerationStructure = trinex_bit(7),

			META_Numeric = trinex_bit(8),
			META_Matrix  = trinex_bit(9),

			META_RW    = trinex_bit(10),
			META_Array = trinex_bit(11),
			META_Cube  = trinex_bit(12),

			// Element type
			META_Boolean     = trinex_bit(13),
			META_Integer     = trinex_bit(14),
			META_Floating    = trinex_bit(15),
			META_Unsigned    = trinex_bit(16),
			META_ElementMask = META_Boolean | META_Integer | META_Floating | META_Unsigned,

			// Dimensions
			META_1R       = trinex_bit(17),
			META_2R       = trinex_bit(18),
			META_3R       = trinex_bit(17) | trinex_bit(18),
			META_4R       = trinex_bit(19),
			META_RowsMask = META_1R | META_2R | META_3R | META_4R,

			META_1C          = trinex_bit(20),
			META_2C          = trinex_bit(21),
			META_3C          = trinex_bit(20) | trinex_bit(21),
			META_4C          = trinex_bit(22),
			META_ColumnsMask = META_1C | META_2C | META_3C | META_4C,

			META_1D            = META_1R | META_1C,
			META_2D            = META_2R | META_2C,
			META_3D            = META_3R | META_3C,
			META_4D            = META_4R | META_4C,
			META_DimensionMask = META_1D | META_2D | META_3D | META_4D,

			META_VaryingInput  = trinex_bit(23),
			META_VaryingOutput = trinex_bit(24),
			META_VaryingMask   = META_VaryingInput | META_VaryingOutput,

			META_Color                    = trinex_bit(25),
			META_ExcludeMaterialParameter = trinex_bit(26),
			META_Concrete                 = trinex_bit(27),

			META_Special0 = trinex_bit(31),
			META_Any      = ~0U,

			// Scalars
			Bool          = META_Concrete | META_Boolean | META_Numeric | META_UniformBuffer | META_1D,
			Bool2         = META_Concrete | META_Boolean | META_Numeric | META_UniformBuffer | META_2D,
			Bool3         = META_Concrete | META_Boolean | META_Numeric | META_UniformBuffer | META_3D,
			Bool4         = META_Concrete | META_Boolean | META_Numeric | META_UniformBuffer | META_4D,
			Int           = META_Concrete | META_Integer | META_Numeric | META_UniformBuffer | META_1D,
			Int2          = META_Concrete | META_Integer | META_Numeric | META_UniformBuffer | META_2D,
			Int3          = META_Concrete | META_Integer | META_Numeric | META_UniformBuffer | META_3D,
			Int4          = META_Concrete | META_Integer | META_Numeric | META_UniformBuffer | META_4D,
			UInt          = META_Concrete | META_Integer | META_Numeric | META_UniformBuffer | META_1D | META_Unsigned,
			UInt2         = META_Concrete | META_Integer | META_Numeric | META_UniformBuffer | META_2D | META_Unsigned,
			UInt3         = META_Concrete | META_Integer | META_Numeric | META_UniformBuffer | META_3D | META_Unsigned,
			UInt4         = META_Concrete | META_Integer | META_Numeric | META_UniformBuffer | META_4D | META_Unsigned,
			Float         = META_Concrete | META_Floating | META_Numeric | META_UniformBuffer | META_1D,
			Float2        = META_Concrete | META_Floating | META_Numeric | META_UniformBuffer | META_2D,
			Float3        = META_Concrete | META_Floating | META_Numeric | META_UniformBuffer | META_3D,
			Float4        = META_Concrete | META_Floating | META_Numeric | META_UniformBuffer | META_4D,
			Float3x3      = META_Concrete | META_Floating | META_Matrix | META_UniformBuffer | META_3D,
			Float4x4      = META_Concrete | META_Floating | META_Matrix | META_UniformBuffer | META_4D,
			UniformBuffer = META_Concrete | META_UniformBuffer,

			// Samplers
			Sampler          = META_Concrete | META_Sampler,
			Sampler1D        = META_Concrete | META_Texture | META_Sampler | META_1D,
			Sampler2D        = META_Concrete | META_Texture | META_Sampler | META_2D,
			Sampler3D        = META_Concrete | META_Texture | META_Sampler | META_3D,
			SamplerCube      = META_Concrete | META_Texture | META_Sampler | META_Cube,
			Sampler1DArray   = META_Concrete | META_Texture | META_Sampler | META_1D | META_Array,
			Sampler2DArray   = META_Concrete | META_Texture | META_Sampler | META_2D | META_Array,
			SamplerCubeArray = META_Concrete | META_Texture | META_Sampler | META_Cube | META_Array,

			RWSampler1D        = META_RW | Sampler1D,
			RWSampler2D        = META_RW | Sampler2D,
			RWSampler3D        = META_RW | Sampler3D,
			RWSamplerCube      = META_RW | SamplerCube,
			RWSampler1DArray   = META_RW | Sampler1DArray,
			RWSampler2DArray   = META_RW | Sampler2DArray,
			RWSamplerCubeArray = META_RW | SamplerCubeArray,

			// Textures
			Texture1D        = META_Concrete | META_Texture | META_1D,
			Texture2D        = META_Concrete | META_Texture | META_2D,
			Texture3D        = META_Concrete | META_Texture | META_3D,
			TextureCube      = META_Concrete | META_Texture | META_Cube,
			Texture1DArray   = META_Concrete | META_Texture | META_1D | META_Array,
			Texture2DArray   = META_Concrete | META_Texture | META_2D | META_Array,
			TextureCubeArray = META_Concrete | META_Texture | META_Cube | META_Array,

			RWTexture1D        = META_RW | Texture1D,
			RWTexture2D        = META_RW | Texture2D,
			RWTexture3D        = META_RW | Texture3D,
			RWTextureCube      = META_RW | TextureCube,
			RWTexture1DArray   = META_RW | Texture1DArray,
			RWTexture2DArray   = META_RW | Texture2DArray,
			RWTextureCubeArray = META_RW | TextureCubeArray,

			// Buffers
			Buffer            = META_Concrete | META_Buffer,
			StructuredBuffer  = META_Concrete | META_StructuredBuffer,
			ByteAddressBuffer = META_Concrete | META_ByteAddressBuffer,

			RWBuffer            = META_RW | Buffer,
			RWStructuredBuffer  = META_RW | StructuredBuffer,
			RWByteAddressBuffer = META_RW | ByteAddressBuffer,

			AccelerationStructure = META_Concrete | META_AccelerationStructure,

			// Descriptors
			DescriptorTexture1D        = Texture1D | META_Descriptor,
			DescriptorTexture2D        = Texture2D | META_Descriptor,
			DescriptorTexture3D        = Texture3D | META_Descriptor,
			DescriptorTextureCube      = TextureCube | META_Descriptor,
			DescriptorTexture1DArray   = Texture1DArray | META_Descriptor,
			DescriptorTexture2DArray   = Texture2DArray | META_Descriptor,
			DescriptorTextureCubeArray = TextureCubeArray | META_Descriptor,

			// Special
			Globals         = UniformBuffer | META_Special0,
			LocalToWorld    = Float4x4 | META_Special0,
			Surface         = Texture2D | META_Special0,
			CombinedSurface = Sampler2D | META_Special0,
		};

		trinex_bitfield_enum_struct(RHIShaderParameterType, EnumerateType);
		trinex_enum(RHIShaderParameterType);

		trinex_api static RHIShaderParameterType make_numeric(RHIShaderParameterType base, u8 len = 1);
		trinex_api static RHIShaderParameterType make_matrix(RHIShaderParameterType base, u8 rows = 4, u8 columns = 4);

		trinex_api RHIShaderParameterType element_type();

		inline constexpr bool is_numeric() const { return (value & META_Numeric); }
		inline constexpr bool is_matrix() const { return (value & META_Matrix) == META_Matrix; }
		inline constexpr bool is_unsigned() const { return value & META_Unsigned; }
		inline constexpr bool is_color() const { return value & META_Color; }
		inline constexpr bool is_concrete() const { return value & META_Concrete; }
		inline constexpr bool is_varying() const { return value & META_VaryingMask; }
		inline constexpr bool is_meta() const { return !is_concrete(); }

		inline constexpr u8 rows() const
		{
			EnumerateType value = bitfield & META_RowsMask;
			return value >> 17;
		}

		inline constexpr u8 columns() const
		{
			EnumerateType value = bitfield & META_ColumnsMask;
			return value >> 20;
		}
	};

	struct RHITextureFlags
	{
		enum Enum : u8
		{
			Undefined          = 0,
			ShaderResource     = trinex_bit(0),
			UnorderedAccess    = trinex_bit(1),
			RenderTarget       = trinex_bit(2),
			DepthStencilTarget = trinex_bit(3),

			ColorAttachment        = RenderTarget | ShaderResource,
			RWColorAttachment      = UnorderedAccess | ColorAttachment,
			RWTexture              = ShaderResource | UnorderedAccess,
			DepthStencilAttachment = DepthStencilTarget | ShaderResource,
		};

		trinex_bitfield_enum_struct(RHITextureFlags, u8);
	};

	struct RHIBufferFlags
	{
		enum Enum : u16
		{
			Undefined         = 0,
			ShaderResource    = trinex_bit(0),
			UnorderedAccess   = trinex_bit(1),
			StructuredBuffer  = trinex_bit(2),
			ByteAddressBuffer = trinex_bit(3),

			VertexBuffer        = trinex_bit(4),
			IndexBuffer         = trinex_bit(5),
			UniformBuffer       = trinex_bit(6),
			IndirectBuffer      = trinex_bit(7),
			TransferSrc         = trinex_bit(8),
			TransferDst         = trinex_bit(9),
			CPURead             = trinex_bit(10),
			CPUWrite            = trinex_bit(11),
			DeviceAddress       = trinex_bit(12),
			AccelerationStorage = trinex_bit(13),
			AccelerationInput   = trinex_bit(14),
			ShaderBindingTable  = trinex_bit(15),
		};

		trinex_bitfield_enum_struct(RHIBufferFlags, u16);
	};

	struct RHIBufferViewType
	{
		enum Enum : u8
		{
			ByteAddress  = 0,
			Structured   = 1,
			Typed        = 2,
			Acceleration = 3,
		};

		trinex_enum_struct(RHIBufferViewType);
	};

	struct RHICompareFunc
	{
		enum Enum : u8
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
		trinex_enum(RHICompareFunc);
	};

	struct RHIAccess
	{
		enum Enum : EnumerateType
		{
			Undefined = 0,

			// Reading states
			CPURead          = trinex_bit(0),
			AccelerationRead = trinex_bit(1),
			IndirectArgs     = trinex_bit(2),
			VertexBuffer     = trinex_bit(3),
			IndexBuffer      = trinex_bit(4),
			UniformBuffer    = trinex_bit(5),
			SRVCompute       = trinex_bit(6),
			SRVGraphics      = trinex_bit(7),
			TransferSrc      = trinex_bit(8),
			PresentSrc       = trinex_bit(9),
			RTVRead          = trinex_bit(10),
			DSVRead          = trinex_bit(11),

			// Writing states
			CPUWrite          = trinex_bit(12),
			AccelerationWrite = trinex_bit(13),
			UAVCompute        = trinex_bit(14),
			UAVGraphics       = trinex_bit(15),
			TransferDst       = trinex_bit(16),
			ResolveDst        = trinex_bit(17),
			RTVWrite          = trinex_bit(18),
			DSVWrite          = trinex_bit(19),

			// Read-Write states
			RTV = RTVRead | RTVWrite,
			DSV = DSVRead | DSVWrite,

			ReadableMask = CPURead | AccelerationRead | IndirectArgs | VertexBuffer | IndexBuffer | UniformBuffer | SRVCompute
			               | SRVGraphics | TransferSrc | PresentSrc | UAVCompute | UAVGraphics | RTVRead | DSVRead,
			WritableMask =
			        CPUWrite | AccelerationWrite | UAVCompute | UAVGraphics | TransferDst | ResolveDst | RTVWrite | DSVWrite,
		};

		trinex_bitfield_enum_struct(RHIAccess, EnumerateType);
	};

	struct RHISemantic
	{
		enum Enum : u8
		{
			Undefined    = 0,
			Position     = 1,
			TexCoord0    = 2,
			TexCoord1    = 3,
			TexCoord2    = 4,
			TexCoord3    = 5,
			Color        = 6,
			Normal       = 7,
			Tangent      = 8,
			Bitangent    = 9,
			BlendWeight  = 10,
			BlendIndices = 11,
			UserData     = 12,
		};

		trinex_enum_struct(RHISemantic);
	};

	struct RHISamplerFilter
	{
		enum Enum : u8
		{
			Point     = 0,
			Bilinear  = 1,
			Trilinear = 2,
		};

		trinex_enum_struct(RHISamplerFilter);
		trinex_enum(RHISamplerFilter);
	};

	struct RHISamplerAddressMode
	{
		enum Enum : u8
		{
			Repeat            = 0,
			ClampToEdge       = 1,
			ClampToBorder     = 2,
			MirroredRepeat    = 3,
			MirrorClampToEdge = 4,
		};

		trinex_enum_struct(RHISamplerAddressMode);
		trinex_enum(RHISamplerAddressMode);
	};

	struct RHIStencilOp
	{
		enum Enum : u8
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
		trinex_enum(RHIStencilOp);
	};

	struct RHIBlendFunc
	{
		enum Enum : u8
		{
			Zero             = 0,
			One              = 1,
			SrcColor         = 2,
			OneMinusSrcColor = 3,
			DstColor         = 4,
			OneMinusDstColor = 5,
			SrcAlpha         = 6,
			OneMinusSrcAlpha = 7,
			DstAlpha         = 8,
			OneMinusDstAlpha = 9,
		};

		trinex_enum_struct(RHIBlendFunc);
		trinex_enum(RHIBlendFunc);
	};

	struct RHIBlendOp
	{
		enum Enum : u8
		{
			Add             = 0,
			Subtract        = 1,
			ReverseSubtract = 2,
			Min             = 3,
			Max             = 4,
		};

		trinex_enum_struct(RHIBlendOp);
		trinex_enum(RHIBlendOp);
	};

	struct RHITopology
	{
		enum Enum : u8
		{
			TriangleList  = 0,
			TriangleStrip = 1,
			LineList      = 2,
			LineStrip     = 3,
		};

		trinex_enum_struct(RHITopology);
		trinex_enum(RHITopology);
	};

	struct RHIPolygonMode
	{
		enum Enum : u8
		{
			Fill = 0,
			Line = 1,
		};

		trinex_enum_struct(RHIPolygonMode);
		trinex_enum(RHIPolygonMode);
	};

	struct RHICullMode
	{
		enum Enum : u8
		{
			None  = 0,
			Front = 1,
			Back  = 2,
		};

		trinex_enum_struct(RHICullMode);
		trinex_enum(RHICullMode);
	};

	struct RHIFrontFace
	{
		enum Enum : u8
		{
			ClockWise        = 0,
			CounterClockWise = 1,
		};

		trinex_enum_struct(RHIFrontFace);
		trinex_enum(RHIFrontFace);
	};

	struct RHIVertexInputRate
	{
		enum Enum : u8
		{
			Vertex   = 0,
			Instance = 1,
		};

		trinex_enum_struct(RHIVertexInputRate);
	};

	struct RHIIndexFormat
	{
		enum Enum : u8
		{
			Undefined = 0,
			UInt16    = 1,
			UInt32    = 2,
		};

		trinex_enum_struct(RHIIndexFormat);

		inline usize stride() const { return value << 1; }
	};

	struct trinex_api RHIColorFormat
	{
		struct Capabilities
		{
			enum Enum : EnumerateType
			{
				Undefined = 0,

				Texture1D   = trinex_bit(0),
				Texture2D   = trinex_bit(1),
				Texture3D   = trinex_bit(2),
				TextureCube = trinex_bit(3),

				RenderTarget   = trinex_bit(4),
				DepthStencil   = trinex_bit(5),
				TextureMipmaps = trinex_bit(6),

				TextureLoad      = trinex_bit(7),
				TextureSample    = trinex_bit(8),
				TextureGather    = trinex_bit(9),
				TextureAtomics   = trinex_bit(10),
				TextureBlendable = trinex_bit(11),
				TextureStore     = trinex_bit(12),
			};

			trinex_bitfield_enum_struct(Capabilities, EnumerateType);
		};

		enum Enum : u8
		{
			Undefined = 0,

			D16_UNORM = 1,
			D24S8     = 2,
			D32F      = 3,

			R8          = 4,
			R8G8        = 5,
			R8G8B8A8    = 7,
			R10G10B10A2 = 8,

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
		trinex_enum(RHIColorFormat);

		constexpr inline RHIColorFormat(const struct RHISurfaceFormat& format);
		constexpr inline RHIColorFormat& operator=(const struct RHISurfaceFormat& format);
		constexpr inline bool operator==(const struct RHISurfaceFormat& format) const noexcept;
		constexpr inline bool operator!=(const struct RHISurfaceFormat& format) const noexcept;

		Capabilities capabilities() const;
		RHIColorFormat add_capabilities(Capabilities capabilities) const;
		RHIColorFormat remove_capabilities(Capabilities capabilities) const;

		constexpr static inline u32 static_count() { return 68; }
		constexpr inline bool is_color() const { return value >= R8 && value <= P010; }
		constexpr inline bool is_depth_stencil() const { return value == D24S8; }
		constexpr inline bool is_depth() const { return value == D32F || value == D16_UNORM; }
		constexpr inline bool has_depth() const { return is_depth() || is_depth_stencil(); }

		template<typename Func>
		constexpr static void static_foreach(Func&& func)
		{
			for (EnumerateType i = 1; i < static_count(); ++i)
			{
				func(RHIColorFormat(static_cast<Enum>(i)));
			}
		}
	};

	struct RHISurfaceFormat
	{
		enum Enum : u8
		{
			Undefined = RHIColorFormat::Undefined,
			D32F      = RHIColorFormat::D32F,
			D16       = RHIColorFormat::D16_UNORM,
			D24S8     = RHIColorFormat::D24S8,

			// Unsigned normalized formats,
			R8      = RHIColorFormat::R8,
			RG8     = RHIColorFormat::R8G8,
			RGBA8   = RHIColorFormat::R8G8B8A8,
			RGB10A2 = RHIColorFormat::R10G10B10A2,
			R16     = RHIColorFormat::R16,
			RG16    = RHIColorFormat::R16G16,
			RGBA16  = RHIColorFormat::R16G16B16A16,

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
		trinex_enum(RHISurfaceFormat);

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
		inline operator RHIColorFormat() const { return as_color_format(); }
	};

	struct RHIVertexFormat
	{
		enum Enum : u8
		{
			Undefined = 0,

			// Unsigned normalized formats,
			R8      = 1,
			RG8     = 2,
			RGB8    = 3,
			RGBA8   = 4,
			RGB10A2 = 5,
			R16     = 6,
			RG16    = 7,
			RGB16   = 8,
			RGBA16  = 9,

			// Signed normalized formats
			R8S      = 10,
			RG8S     = 11,
			RGB8S    = 12,
			RGBA8S   = 13,
			RGB10A2S = 14,
			R16S     = 15,
			RG16S    = 16,
			RGB16S   = 17,
			RGBA16S  = 18,

			// Unsigned integer formats
			R8UI      = 19,
			RG8UI     = 20,
			RGB8UI    = 21,
			RGBA8UI   = 22,
			RGB10A2UI = 23,
			R16UI     = 24,
			RG16UI    = 25,
			RGB16UI   = 26,
			RGBA16UI  = 27,
			R32UI     = 28,
			RG32UI    = 29,
			RGB32UI   = 30,
			RGBA32UI  = 31,

			// Signed integer formats
			R8SI      = 32,
			RG8SI     = 33,
			RGB8SI    = 34,
			RGBA8SI   = 35,
			RGB10A2SI = 36,
			R16SI     = 37,
			RG16SI    = 38,
			RGB16SI   = 39,
			RGBA16SI  = 40,
			R32SI     = 41,
			RG32SI    = 42,
			RGB32SI   = 43,
			RGBA32SI  = 44,

			// Floating formats
			R16F    = 45,
			RG16F   = 46,
			RGB16F  = 47,
			RGBA16F = 48,
			R32F    = 49,
			RG32F   = 50,
			RGB32F  = 51,
			RGBA32F = 52,
		};

		trinex_enum_struct(RHIVertexFormat);
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

	struct RHIColorComponent
	{
		enum Enum : u8
		{
			Undefined = 0,
			R         = 1,
			G         = 2,
			B         = 4,
			A         = 8,

			RG   = R | G,
			RGB  = R | G | B,
			RGBA = R | G | B | A,
		};

		trinex_bitfield_enum_struct(RHIColorComponent, u8);
		trinex_enum(RHIColorComponent);
	};

	struct RHITextureType
	{
		enum Enum : u8
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

	struct RHICubeFace
	{
		enum Enum : u8
		{
			Right  = 0,
			Left   = 1,
			Top    = 2,
			Bottom = 3,
			Front  = 4,
			Back   = 5,
		};
		trinex_enum_struct(RHICubeFace);
	};

	struct RHILoadFunc
	{
		enum Enum : u8
		{
			Load     = 0,
			Clear    = 1,
			DontCare = 2,
		};

		trinex_enum_struct(RHILoadFunc);
	};

	struct RHIStoreFunc
	{
		enum Enum : u8
		{
			Store    = 0,
			DontCare = 1,
		};

		trinex_enum_struct(RHIStoreFunc);
	};

	struct RHIResolveFunc
	{
		enum Enum : u8
		{
			Undefined = 0,
			Sample0   = 1,
			Average   = 2,
			Min       = 3,
			Max       = 4,
		};

		trinex_enum_struct(RHIResolveFunc);
	};

	struct RHIRenderingFlags
	{
		enum Enum : u8
		{
			Undefined            = 0,
			SecondaryBuffersOnly = 1 << 0,
			Suspending           = 1 << 1,
			Resuming             = 1 << 2,
		};

		trinex_bitfield_enum_struct(RHIRenderingFlags, u8);
	};

	struct RHIAspect
	{
		enum Enum : u8
		{
			Undefined    = 0,
			Color        = 1 << 0,
			Depth        = 1 << 1,
			Stencil      = 1 << 2,
			DepthStencil = Depth | Stencil,
		};

		trinex_bitfield_enum_struct(RHIAspect, u8);
	};

	struct RHIContextInheritanceFlags
	{
		enum Enum : u8
		{
			Undefined          = 0,
			RenderPassContinue = 1 << 0,
		};

		trinex_bitfield_enum_struct(RHIContextInheritanceFlags, u8);
	};

	struct RHIContextFlags
	{
		enum Enum : u8
		{
			Undefined = 0,
			Secondary = 1 << 0,
			Graphics  = 1 << 2,
			Compute   = 1 << 3,
			Transfer  = 1 << 4,
		};

		trinex_bitfield_enum_struct(RHIContextFlags, u8);
	};

	struct RHIMappingAccess
	{
		enum Enum : u8
		{
			Undefined = 0,
			Read      = 1 << 0,
			Write     = 1 << 1,
		};

		trinex_enum_struct(RHIMappingAccess);
	};

	struct RHIBorderColor
	{
		enum Enum : u8
		{
			FloatOpaqueBlack      = 0,
			FloatOpaqueWhite      = 1,
			FloatTransparentBlack = 2,

			IntOpaqueBlack      = 3,
			IntOpaqueWhite      = 4,
			IntTransparentBlack = 5,
		};

		trinex_enum_struct(RHIBorderColor);
		trinex_enum(RHIBorderColor);
	};

	struct RHIRayTracingGeometryType
	{
		enum Enum : u8
		{
			Triangles = 0,
			AABBs     = 1,
		};

		trinex_enum_struct(RHIRayTracingGeometryType);
	};

	struct RHIRayTracingAccelerationLevel
	{
		enum Enum : u8
		{
			Top    = 0,
			Bottom = 1,
		};

		trinex_enum_struct(RHIRayTracingAccelerationLevel);
	};

	struct RHIRayTracingGeometryFlags
	{
		enum Enum : u8
		{
			Undefined         = 0,
			Opaque            = trinex_bit(0),
			NoDuplicateAnyHit = trinex_bit(1),
		};

		trinex_bitfield_enum_struct(RHIRayTracingGeometryFlags, u8);
	};

	struct RHIRayTracingAccelerationFlags
	{
		enum Enum : u8
		{
			Undefined       = 0,
			AllowUpdate     = trinex_bit(0),
			AllowCompaction = trinex_bit(1),
			PreferFastTrace = trinex_bit(2),
			PreferFastBuild = trinex_bit(3),
			MinimizeMemory  = trinex_bit(4),
			PerformUpdate   = trinex_bit(5),
		};

		trinex_bitfield_enum_struct(RHIRayTracingAccelerationFlags, u8);
	};

	struct RHIRayTracingShaderGroupType
	{
		enum Enum : u8
		{
			GeneralRayGen   = 0,
			GeneralMiss     = 1,
			GeneralCallable = 2,
			TrianglesHit    = 3,
			ProceduralHit   = 4,
		};

		trinex_enum_struct(RHIRayTracingShaderGroupType);
	};

	struct RHIRayTracingGeometryInstanceFlags
	{
		enum Enum : u8
		{
			Undefined                 = 0,
			TriangleFacingCullDisable = trinex_bit(0),
			TriangleFlipFacing        = trinex_bit(1),
			TriangleFrontCCW          = trinex_bit(2),
			ForceOpaque               = trinex_bit(3),
			ForceNoOpaque             = trinex_bit(4),
		};

		trinex_bitfield_enum_struct(RHIRayTracingGeometryInstanceFlags, u8);
	};
} // namespace Trinex
