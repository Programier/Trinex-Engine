#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	struct ENGINE_EXPORT ColorFormat {
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

			Depth        = 1,
			DepthStencil = 2,
			ShadowDepth  = 3,

			R8           = 4,
			R8G8         = 5,
			R8G8B8       = 6,
			R8G8B8A8     = 7,
			R10G108B10A2 = 8,

			R8_SNORM       = 9,
			R8G8_SNORM     = 10,
			R8G8B8_SNORM   = 11,
			R8G8B8A8_SNORM = 12,

			R8_UINT       = 13,
			R8G8_UINT     = 14,
			R8G8B8_UINT   = 15,
			R8G8B8A8_UINT = 16,

			R8_SINT       = 17,
			R8G8_SINT     = 18,
			R8G8B8_SINT   = 19,
			R8G8B8A8_SINT = 20,

			R16          = 21,
			R16G16       = 22,
			R16G16B16    = 23,
			R16G16B16A16 = 24,

			R16_SNORM          = 25,
			R16G16_SNORM       = 26,
			R16G16B16_SNORM    = 27,
			R16G16B16A16_SNORM = 28,

			R16_UINT          = 29,
			R16G16_UINT       = 30,
			R16G16B16_UINT    = 31,
			R16G16B16A16_UINT = 32,

			R16_SINT          = 33,
			R16G16_SINT       = 34,
			R16G16B16_SINT    = 35,
			R16G16B16A16_SINT = 36,

			R32_UINT          = 37,
			R32G32_UINT       = 38,
			R32G32B32_UINT    = 39,
			R32G32B32A32_UINT = 40,

			R32_SINT          = 41,
			R32G32_SINT       = 42,
			R32G32B32_SINT    = 43,
			R32G32B32A32_SINT = 44,

			R16F          = 45,
			R16G16F       = 46,
			R16G16B16F    = 47,
			R16G16B16A16F = 48,

			R32F          = 49,
			R32G32F       = 50,
			R32G32B32F    = 51,
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

		trinex_enum_struct(ColorFormat);
		trinex_declare_enum(ColorFormat);

		Capabilities capabilities() const;
		ColorFormat add_capabilities(Capabilities capabilities) const;
		ColorFormat remove_capabilities(Capabilities capabilities) const;

		constexpr static inline uint32_t static_count() { return 68; }
		constexpr inline bool is_color() const { return value >= R8 && value <= P010; }
		constexpr inline bool is_depth_stencil() const { return value == DepthStencil; }
		constexpr inline bool is_depth() const { return value == Depth || value == ShadowDepth; }
		constexpr inline bool has_depth() const { return value >= Depth && value <= ShadowDepth; }

		template<typename Func>
		constexpr static void static_foreach(Func&& func)
		{
			for (EnumerateType i = 1; i < static_count(); ++i)
			{
				func(ColorFormat(static_cast<Enum>(i)));
			}
		}
	};

	struct SurfaceFormat {
		enum Enum : EnumerateType
		{
			Undefined   = ColorFormat::Undefined,
			Depth       = ColorFormat::Depth,
			ShadowDepth = ColorFormat::ShadowDepth,

			R8       = ColorFormat::R8,
			RG8      = ColorFormat::R8G8,
			RGBA8    = ColorFormat::R8G8B8A8,
			RG8B10A2 = ColorFormat::R10G108B10A2,

			R16F    = ColorFormat::R16F,
			RG16F   = ColorFormat::R16G16F,
			RGBA16F = ColorFormat::R16G16B16A16F,
		};

		trinex_enum_struct(SurfaceFormat);
		trinex_declare_enum(SurfaceFormat);

		inline ColorFormat as_color_format() const { return static_cast<ColorFormat::Enum>(value); }

		inline bool is_color() const { return as_color_format().is_color(); }
		inline bool is_depth_stencil() const { return as_color_format().is_depth_stencil(); }
		inline bool is_depth() const { return as_color_format().is_depth(); }
		inline bool has_depth() const { return as_color_format().has_depth(); }

		operator ColorFormat() const { return as_color_format(); }
	};
}// namespace Engine
