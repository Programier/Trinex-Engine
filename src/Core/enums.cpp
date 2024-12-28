#include <Core/engine_loading_controllers.hpp>
#include <Core/enums.hpp>
#include <Core/reflection/enum.hpp>

namespace Engine
{
	implement_engine_enum(TextureType, TextureType::Texture2D, TextureType::TextureCubeMap);
	implement_engine_enum(CompareMode, CompareMode::None, CompareMode::RefToTexture);
	implement_engine_enum(SamplerFilter, SamplerFilter::Point, SamplerFilter::Bilinear, SamplerFilter::Trilinear);
	implement_engine_enum(Swizzle, Swizzle::Identity, Swizzle::Zero, Swizzle::One, Swizzle::R, Swizzle::G, Swizzle::B,
						  Swizzle::A);

	implement_engine_enum(SamplerAddressMode, SamplerAddressMode::Repeat, SamplerAddressMode::ClampToEdge,
						  SamplerAddressMode::ClampToBorder, SamplerAddressMode::MirroredRepeat,
						  SamplerAddressMode::MirrorClampToEdge);
	implement_engine_enum(TextureCubeMapFace, TextureCubeMapFace::Front, TextureCubeMapFace::Back, TextureCubeMapFace::Up,
						  TextureCubeMapFace::Down, TextureCubeMapFace::Left, TextureCubeMapFace::Right);

	implement_engine_enum(VertexBufferSemantic, VertexBufferSemantic::Position, VertexBufferSemantic::TexCoord,
						  VertexBufferSemantic::Color, VertexBufferSemantic::Normal, VertexBufferSemantic::Tangent,
						  VertexBufferSemantic::Bitangent, VertexBufferSemantic::BlendWeight, VertexBufferSemantic::BlendIndices);

	implement_engine_enum(Coord, Coord::X, Coord::Y, Coord::Z);

	implement_engine_enum(DataType, DataType::Text, DataType::Binary);

	implement_engine_enum(OperationSystemType, OperationSystemType::Linux, OperationSystemType::Windows,
						  OperationSystemType::Android);

	implement_engine_enum(ColorComponent, ColorComponent::R, ColorComponent::G, ColorComponent::B, ColorComponent::A);

	implement_engine_enum(CompareFunc, CompareFunc::Always, CompareFunc::Lequal, CompareFunc::Gequal, CompareFunc::Less,
						  CompareFunc::Greater, CompareFunc::Equal, CompareFunc::NotEqual, CompareFunc::Never);

	implement_engine_enum(PhysicalSizeMetric, PhysicalSizeMetric::Inch, PhysicalSizeMetric::Сentimeters);

	implement_engine_enum(StencilOp, StencilOp::Keep, StencilOp::Zero, StencilOp::Replace, StencilOp::Incr, StencilOp::IncrWrap,
						  StencilOp::Decr, StencilOp::DecrWrap, StencilOp::Invert);

	implement_engine_enum(BlendFunc, BlendFunc::Zero, BlendFunc::One, BlendFunc::SrcColor, BlendFunc::OneMinusSrcColor,
						  BlendFunc::DstColor, BlendFunc::OneMinusDstColor, BlendFunc::SrcAlpha, BlendFunc::OneMinusSrcAlpha,
						  BlendFunc::DstAlpha, BlendFunc::OneMinusDstAlpha, BlendFunc::BlendFactor,
						  BlendFunc::OneMinusBlendFactor);

	implement_engine_enum(BlendOp, BlendOp::Add, BlendOp::Subtract, BlendOp::ReverseSubtract, BlendOp::Min, BlendOp::Max);

	implement_engine_enum(Primitive, Primitive::Triangle, Primitive::Line, Primitive::Point);

	implement_engine_enum(PrimitiveTopology, PrimitiveTopology::TriangleList, PrimitiveTopology::PointList,
						  PrimitiveTopology::LineList, PrimitiveTopology::LineStrip, PrimitiveTopology::TriangleStrip);

	implement_engine_enum(PolygonMode, PolygonMode::Fill, PolygonMode::Line, PolygonMode::Point);

	implement_engine_enum(CullMode, CullMode::None, CullMode::Front, CullMode::Back);

	implement_engine_enum(FrontFace, FrontFace::ClockWise, FrontFace::CounterClockWise);

	implement_engine_enum(WindowAttribute, WindowAttribute::None, WindowAttribute::Resizable, WindowAttribute::FullScreen,
						  WindowAttribute::Shown, WindowAttribute::Hidden, WindowAttribute::BorderLess,
						  WindowAttribute::MouseFocus, WindowAttribute::InputFocus, WindowAttribute::InputGrabbed,
						  WindowAttribute::Minimized, WindowAttribute::Maximized, WindowAttribute::MouseCapture,
						  WindowAttribute::MouseGrabbed, WindowAttribute::KeyboardGrabbed);

	implement_engine_enum(CursorMode, CursorMode::Normal, CursorMode::Hidden);

	implement_engine_enum(Orientation, Orientation::Landscape, Orientation::LandscapeFlipped, Orientation::Portrait,
						  Orientation::PortraitFlipped);

	implement_engine_enum(MessageBoxType, MessageBoxType::Error, MessageBoxType::Warning, MessageBoxType::Info);

	implement_engine_enum(VertexAttributeInputRate, VertexAttributeInputRate::Vertex, VertexAttributeInputRate::Instance);

	implement_engine_enum(ColorComponentMask, ColorComponentMask::RGBA, ColorComponentMask::RGB, ColorComponentMask::RGA,
						  ColorComponentMask::RG, ColorComponentMask::RBA, ColorComponentMask::RB, ColorComponentMask::RA,
						  ColorComponentMask::R, ColorComponentMask::GBA, ColorComponentMask::GB, ColorComponentMask::GA,
						  ColorComponentMask::G, ColorComponentMask::BA, ColorComponentMask::B, ColorComponentMask::A);

	implement_engine_enum(RenderPassType, RenderPassType::Undefined, RenderPassType::Window, RenderPassType::SceneColor,
						  RenderPassType::GBuffer);

	implement_engine_enum(ViewMode, ViewMode::Lit, ViewMode::Unlit, ViewMode::Wireframe, ViewMode::WorldNormal, ViewMode::Metalic,
						  ViewMode::Roughness, ViewMode::Specular, ViewMode::AO);

	implement_engine_enum(VertexBufferElementType, VertexBufferElementType::Undefined, VertexBufferElementType::Float1,
						  VertexBufferElementType::Float2, VertexBufferElementType::Float3, VertexBufferElementType::Float4,
						  VertexBufferElementType::Byte1, VertexBufferElementType::Byte2, VertexBufferElementType::Byte4,
						  VertexBufferElementType::Byte1, VertexBufferElementType::Byte2N, VertexBufferElementType::Byte4N,
						  VertexBufferElementType::UByte1, VertexBufferElementType::UByte2, VertexBufferElementType::UByte4,
						  VertexBufferElementType::UByte1N, VertexBufferElementType::UByte2N, VertexBufferElementType::UByte4N,
						  VertexBufferElementType::Color, VertexBufferElementType::Short1, VertexBufferElementType::Short2,
						  VertexBufferElementType::Short4, VertexBufferElementType::Short1N, VertexBufferElementType::Short2N,
						  VertexBufferElementType::Short4N, VertexBufferElementType::UShort1, VertexBufferElementType::UShort2,
						  VertexBufferElementType::UShort4, VertexBufferElementType::UShort1N, VertexBufferElementType::UShort2N,
						  VertexBufferElementType::UShort4N, VertexBufferElementType::Int1, VertexBufferElementType::Int2,
						  VertexBufferElementType::Int3, VertexBufferElementType::Int4, VertexBufferElementType::UInt1,
						  VertexBufferElementType::UInt2, VertexBufferElementType::UInt3, VertexBufferElementType::UInt4);

	implement_engine_enum(ColorFormat, ColorFormat::Undefined, ColorFormat::FloatR, ColorFormat::FloatRGBA, ColorFormat::R8,
						  ColorFormat::R8G8B8A8, ColorFormat::Depth, ColorFormat::DepthStencil, ColorFormat::ShadowDepth,
						  ColorFormat::BC1, ColorFormat::BC2, ColorFormat::BC3);

	implement_engine_enum(MaterialDomain, MaterialDomain::Surface);


	implement_engine_enum(SplashTextType, SplashTextType::StartupProgress, SplashTextType::VersionInfo,
						  SplashTextType::CopyrightInfo, SplashTextType::GameName);

}// namespace Engine
