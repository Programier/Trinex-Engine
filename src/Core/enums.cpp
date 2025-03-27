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

	trinex_implement_engine_enum(ColorFormat, Undefined, FloatR, FloatRGBA, R8, R8G8B8A8, Depth, DepthStencil, ShadowDepth, BC1,
								 BC2, BC3);

	trinex_implement_engine_enum(MaterialDomain, Surface, Lighting);
	trinex_implement_engine_enum(SplashTextType, StartupProgress, VersionInfo, CopyrightInfo, GameName);
	trinex_implement_engine_enum(MaterialOptions, DefaultPassOnly, DisableDefaultPass, LightMaterial);

}// namespace Engine
