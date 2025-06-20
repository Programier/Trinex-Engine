#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/enum.hpp>
#include <RHI/enums.hpp>

namespace Engine
{
	trinex_implement_engine_enum(RHISamplerFilter, Point, Bilinear, Trilinear);
	trinex_implement_engine_enum(RHISamplerAddressMode, Repeat, ClampToEdge, ClampToBorder, MirroredRepeat, MirrorClampToEdge);
	trinex_implement_engine_enum(RHIColorComponent, R, G, B, A);
	trinex_implement_engine_enum(RHICompareFunc, Always, Lequal, Gequal, Less, Greater, Equal, NotEqual, Never);
	trinex_implement_engine_enum(RHIStencilOp, Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert);
	trinex_implement_engine_enum(RHIBlendFunc, Zero, One, SrcColor, OneMinusSrcColor, DstColor, OneMinusDstColor, SrcAlpha,
	                             OneMinusSrcAlpha, DstAlpha, OneMinusDstAlpha, BlendFactor, OneMinusBlendFactor);
	trinex_implement_engine_enum(RHIBlendOp, Add, Subtract, ReverseSubtract, Min, Max);
	trinex_implement_engine_enum(RHIPrimitive, Triangle, Line, Point);
	trinex_implement_engine_enum(RHIPrimitiveTopology, TriangleList, PointList, LineList, LineStrip, TriangleStrip);
	trinex_implement_engine_enum(RHIPolygonMode, Fill, Line, Point);
	trinex_implement_engine_enum(RHICullMode, None, Front, Back);
	trinex_implement_engine_enum(RHIFrontFace, ClockWise, CounterClockWise);
	trinex_implement_engine_enum(RHIVertexAttributeInputRate, Vertex, Instance);
	trinex_implement_engine_enum(RHIShaderParameterType, Undefined, META_UniformBuffer, META_Sampler, META_Texture, META_Scalar,
	                             META_Vector, META_Matrix, META_Numeric, META_Any, Bool, Bool2, Bool3, Bool4, Int, Int2, Int3,
	                             Int4, UInt, UInt2, UInt3, UInt4, Float, Float2, Float3, Float4, Float3x3, Float4x4, MemoryBlock,
	                             Sampler, Sampler2D, Texture2D);

	trinex_implement_engine_enum(RHIColorFormat, Undefined, R8, R8G8, R8G8B8A8, R8_SNORM, R8G8_SNORM, R8G8B8A8_SNORM, R8_UINT,
	                             R8G8_UINT, R8G8B8A8_UINT, R8_SINT, R8G8_SINT, R8G8B8A8_SINT, R16, R16G16, R16G16B16A16,
	                             R16_SNORM, R16G16_SNORM, R16G16B16A16_SNORM, R16_UINT, R16G16_UINT, R16G16B16A16_UINT, R16_SINT,
	                             R16G16_SINT, R16G16B16A16_SINT, R32_UINT, R32G32_UINT, R32G32B32A32_UINT, R32_SINT, R32G32_SINT,
	                             R32G32B32A32_SINT, R16F, R16G16F, R16G16B16A16F, R32F, R32G32F, R32G32B32A32F, BC1_RGBA,
	                             BC2_RGBA, BC3_RGBA, BC4_R, BC5_RG, BC7_RGBA, ASTC_4x4_RGBA, ASTC_6x6_RGBA, ASTC_8x8_RGBA,
	                             ASTC_10x10_RGBA, ETC1_RGB, ETC2_RGB, ETC2_RGBA, NV12, P010, Depth, DepthStencil, ShadowDepth);

	trinex_implement_engine_enum(RHISurfaceFormat, Undefined, Depth, ShadowDepth, R8, RG8, RGBA8, RG8B10A2, R16F, RG16F, RGBA16F);

	static RHIColorFormat::Capabilities s_capabilities[RHIColorFormat::static_count()] = {};

	RHIColorFormat::Capabilities RHIColorFormat::capabilities() const
	{
		return s_capabilities[value];
	}

	RHIColorFormat RHIColorFormat::add_capabilities(Capabilities capabilities) const
	{
		s_capabilities[value] |= capabilities;
		return *this;
	}

	RHIColorFormat RHIColorFormat::remove_capabilities(Capabilities capabilities) const
	{
		s_capabilities[value] &= ~capabilities;
		return *this;
	}

	ENGINE_EXPORT RHIShaderParameterType RHIShaderParameterType::make_vector(byte len)
	{
		len = glm::clamp<byte>(len, 1, 4);

		if (is_scalar())
		{
			if (len == 1)
				return *this;

			RHIShaderParameterType result = *this;
			result &= RHIShaderParameterType(~META_Scalar);
			result |= META_Vector;
			result.bitfield += static_cast<EnumerateType>(len - 1);
			return result;
		}
		else if (is_vector())
		{
			RHIShaderParameterType result = *this;
			byte current_len              = vector_length();

			if (current_len > len)
			{
				result.bitfield -= static_cast<EnumerateType>(current_len - len);

				if (len == 1)
				{
					result &= RHIShaderParameterType(~META_Vector);
					result |= META_Scalar;
				}
			}
			else if (current_len < len)
			{
				result.bitfield += static_cast<EnumerateType>(len - current_len);
			}
			return result;
		}

		return RHIShaderParameterType();
	}

	ENGINE_EXPORT RHIShaderParameterType RHIShaderParameterType::make_scalar()
	{
		if (is_scalar())
			return *this;

		if (is_vector())
		{
			RHIShaderParameterType result = *this;
			byte current_len              = vector_length();

			result.bitfield -= static_cast<EnumerateType>(current_len - 1);
			result &= RHIShaderParameterType(~META_Vector);
			result |= META_Scalar;

			return result;
		}

		return RHIShaderParameterType();
	}
}// namespace Engine
