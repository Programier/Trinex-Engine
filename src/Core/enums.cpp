#include <Core/engine_loading_controllers.hpp>
#include <Core/enums.hpp>
#include <Core/reflection/enum.hpp>

namespace Engine
{
	trinex_implement_enum(TextureType, Texture2D, TextureCubeMap);

	trinex_implement_engine_enum(CompareMode, None, RefToTexture);
	trinex_implement_engine_enum(SamplerFilter, Point, Bilinear, Trilinear);
	trinex_implement_engine_enum(SamplerAddressMode, Repeat, ClampToEdge, ClampToBorder, MirroredRepeat, MirrorClampToEdge);
	trinex_implement_engine_enum(TextureCubeMapFace, Front, Back, Up, Down, Left, Right);

	trinex_implement_engine_enum(VertexBufferSemantic, Position, TexCoord, Color, Normal, Tangent, Bitangent, BlendWeight,
	                             BlendIndices);

	trinex_implement_engine_enum(Coord, X, Y, Z);
	trinex_implement_engine_enum(DataType, Text, Binary);
	trinex_implement_engine_enum(OperationSystemType, Linux, Windows, Android);
	trinex_implement_engine_enum(ColorComponent, R, G, B, A);
	trinex_implement_engine_enum(CompareFunc, Always, Lequal, Gequal, Less, Greater, Equal, NotEqual, Never);
	trinex_implement_engine_enum(PhysicalSizeMetric, Inch, Сentimeters);
	trinex_implement_engine_enum(StencilOp, Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert);

	trinex_implement_engine_enum(BlendFunc, Zero, One, SrcColor, OneMinusSrcColor, DstColor, OneMinusDstColor, SrcAlpha,
	                             OneMinusSrcAlpha, DstAlpha, OneMinusDstAlpha, BlendFactor, OneMinusBlendFactor);

	trinex_implement_engine_enum(BlendOp, Add, Subtract, ReverseSubtract, Min, Max);
	trinex_implement_engine_enum(Primitive, Triangle, Line, Point);
	trinex_implement_engine_enum(PrimitiveTopology, TriangleList, PointList, LineList, LineStrip, TriangleStrip);
	trinex_implement_engine_enum(PolygonMode, Fill, Line, Point);
	trinex_implement_engine_enum(CullMode, None, Front, Back);
	trinex_implement_engine_enum(FrontFace, ClockWise, CounterClockWise);

	trinex_implement_engine_enum(WindowAttribute, None, Resizable, FullScreen, Shown, Hidden, BorderLess, MouseFocus, InputFocus,
	                             InputGrabbed, Minimized, Maximized, MouseCapture, MouseGrabbed, KeyboardGrabbed);

	trinex_implement_engine_enum(CursorMode, Normal, Hidden);
	trinex_implement_engine_enum(Orientation, Landscape, LandscapeFlipped, Portrait, PortraitFlipped);
	trinex_implement_engine_enum(MessageBoxType, Error, Warning, Info);
	trinex_implement_engine_enum(VertexAttributeInputRate, Vertex, Instance);
	trinex_implement_engine_enum(RenderPassType, Undefined, Window, SceneColor, GBuffer);
	trinex_implement_engine_enum(ViewMode, Lit, Unlit, Wireframe, WorldNormal, Metalic, Roughness, Specular, AO);

	trinex_implement_engine_enum(VertexBufferElementType, Undefined, Float1, Float2, Float3, Float4, Byte1, Byte2, Byte4, Byte1,
	                             Byte2N, Byte4N, UByte1, UByte2, UByte4, UByte1N, UByte2N, UByte4N, Color, Short1, Short2, Short4,
	                             Short1N, Short2N, Short4N, UShort1, UShort2, UShort4, UShort1N, UShort2N, UShort4N, Int1, Int2,
	                             Int3, Int4, UInt1, UInt2, UInt3, UInt4);

	trinex_implement_engine_enum(MaterialDomain, Surface, PostProcess);
	trinex_implement_engine_enum(SplashTextType, StartupProgress, VersionInfo, CopyrightInfo, GameName);

	trinex_implement_engine_enum(ShaderParameterType, Undefined, META_UniformBuffer, META_Sampler, META_Texture, META_Scalar,
	                             META_Vector, META_Matrix, META_Numeric, META_Any, Bool, Bool2, Bool3, Bool4, Int, Int2, Int3,
	                             Int4, UInt, UInt2, UInt3, UInt4, Float, Float2, Float3, Float4, Float3x3, Float4x4, MemoryBlock,
	                             Sampler, Sampler2D, Texture2D);

	ShaderParameterType ShaderParameterType::make_vector(byte len)
	{
		len = glm::clamp<byte>(len, 1, 4);

		if (is_scalar())
		{
			if (len == 1)
				return *this;

			ShaderParameterType result = *this;
			result &= ShaderParameterType(~META_Scalar);
			result |= META_Vector;
			result.bitfield += static_cast<EnumerateType>(len - 1);
			return result;
		}
		else if (is_vector())
		{
			ShaderParameterType result = *this;
			byte current_len           = vector_length();

			if (current_len > len)
			{
				result.bitfield -= static_cast<EnumerateType>(current_len - len);

				if (len == 1)
				{
					result &= ShaderParameterType(~META_Vector);
					result |= META_Scalar;
				}
			}
			else if (current_len < len)
			{
				result.bitfield += static_cast<EnumerateType>(len - current_len);
			}
			return result;
		}

		return ShaderParameterType();
	}

	ShaderParameterType ShaderParameterType::make_scalar()
	{
		if (is_scalar())
			return *this;

		if (is_vector())
		{
			ShaderParameterType result = *this;
			byte current_len           = vector_length();

			result.bitfield -= static_cast<EnumerateType>(current_len - 1);
			result &= ShaderParameterType(~META_Vector);
			result |= META_Scalar;

			return result;
		}

		return ShaderParameterType();
	}
}// namespace Engine
