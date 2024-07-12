#include <Core/enum.hpp>
#include <Core/enums.hpp>

namespace Engine
{
    implement_enum(TextureType, Engine, {"Texture2D", TextureType::Texture2D}, {"TextureCubeMap", TextureType::TextureCubeMap});
    implement_enum(CompareMode, Engine, {"None", CompareMode::None}, {"RefToTexture", CompareMode::RefToTexture});
    implement_enum(SamplerFilter, Engine, {"Point", SamplerFilter::Point}, {"Bilinear", SamplerFilter::Bilinear},
                   {"Trilinear", SamplerFilter::Trilinear});
    implement_enum(Swizzle, Engine, {"Identity", Swizzle::Identity}, {"Zero", Swizzle::Zero}, {"One", Swizzle::One},
                   {"R", Swizzle::R}, {"G", Swizzle::G}, {"B", Swizzle::B}, {"A", Swizzle::A});

    implement_enum(SamplerAddressMode, Engine, {"Repeat", SamplerAddressMode::Repeat},
                   {"ClampToEdge", SamplerAddressMode::ClampToEdge}, {"ClampToBorder", SamplerAddressMode::ClampToBorder},
                   {"MirroredRepeat", SamplerAddressMode::MirroredRepeat},
                   {"MirrorClampToEdge", SamplerAddressMode::MirrorClampToEdge});

    implement_enum(TextureCubeMapFace, Engine, {"Front", TextureCubeMapFace::Front}, {"Back", TextureCubeMapFace::Back},
                   {"Up", TextureCubeMapFace::Up}, {"Down", TextureCubeMapFace::Down}, {"Left", TextureCubeMapFace::Left},
                   {"Right", TextureCubeMapFace::Right});

    implement_enum(VertexBufferSemantic, Engine, {"Position", VertexBufferSemantic::Position},
                   {"TexCoord", VertexBufferSemantic::TexCoord}, {"Color", VertexBufferSemantic::Color},
                   {"Normal", VertexBufferSemantic::Normal}, {"Tangent", VertexBufferSemantic::Tangent},
                   {"Binormal", VertexBufferSemantic::Binormal}, {"BlendWeight", VertexBufferSemantic::BlendWeight},
                   {"BlendIndices", VertexBufferSemantic::BlendIndices});

    implement_enum(Coord, Engine, {"X", Coord::X}, {"Y", Coord::Y}, {"Z", Coord::Z});
    implement_enum(DataType, Engine, {"Text", DataType::Text}, {"Binary", DataType::Binary});

    implement_enum(OperationSystemType, Engine, {"Linux", OperationSystemType::Linux}, {"Windows", OperationSystemType::Windows},
                   {"Android", OperationSystemType::Android});

    implement_enum(ColorComponent, Engine, {"R", ColorComponent::R}, {"G", ColorComponent::G}, {"B", ColorComponent::B},
                   {"A", ColorComponent::A});

    implement_enum(CompareFunc, Engine, {"Always", CompareFunc::Always}, {"Lequal", CompareFunc::Lequal},
                   {"Gequal", CompareFunc::Gequal}, {"Less", CompareFunc::Less}, {"Greater", CompareFunc::Greater},
                   {"Equal", CompareFunc::Equal}, {"NotEqual", CompareFunc::NotEqual}, {"Never", CompareFunc::Never});

    implement_enum(PhysicalSizeMetric, Engine, {"Inch", PhysicalSizeMetric::Inch},
                   {"Сentimeters", PhysicalSizeMetric::Сentimeters});


    implement_enum(StencilOp, Engine, {"Keep", StencilOp::Keep}, {"Zero", StencilOp::Zero}, {"Replace", StencilOp::Replace},
                   {"Incr", StencilOp::Incr}, {"IncrWrap", StencilOp::IncrWrap}, {"Decr", StencilOp::Decr},
                   {"DecrWrap", StencilOp::DecrWrap}, {"Invert", StencilOp::Invert});

    implement_enum(BlendFunc, Engine, {"Zero", BlendFunc::Zero}, {"One", BlendFunc::One}, {"SrcColor", BlendFunc::SrcColor},
                   {"OneMinusSrcColor", BlendFunc::OneMinusSrcColor}, {"DstColor", BlendFunc::DstColor},
                   {"OneMinusDstColor", BlendFunc::OneMinusDstColor}, {"SrcAlpha", BlendFunc::SrcAlpha},
                   {"OneMinusSrcAlpha", BlendFunc::OneMinusSrcAlpha}, {"DstAlpha", BlendFunc::DstAlpha},
                   {"OneMinusDstAlpha", BlendFunc::OneMinusDstAlpha}, {"BlendFactor", BlendFunc::BlendFactor},
                   {"OneMinusBlendFactor", BlendFunc::OneMinusBlendFactor});

    implement_enum(BlendOp, Engine, {"Add", BlendOp::Add}, {"Subtract", BlendOp::Subtract},
                   {"ReverseSubtract", BlendOp::ReverseSubtract}, {"Min", BlendOp::Min}, {"Max", BlendOp::Max});

    implement_enum(Primitive, Engine, {"Triangle", Primitive::Triangle}, {"Line", Primitive::Line}, {"Point", Primitive::Point});

    implement_enum(DepthFunc, Engine, {"Always", DepthFunc::Always}, {"Lequal", DepthFunc::Lequal}, {"Gequal", DepthFunc::Gequal},
                   {"Less", DepthFunc::Less}, {"Greater", DepthFunc::Greater}, {"Equal", DepthFunc::Equal},
                   {"NotEqual", DepthFunc::NotEqual}, {"Never", DepthFunc::Never});

    implement_enum(PrimitiveTopology, Engine, {"TriangleList", PrimitiveTopology::TriangleList},
                   {"PointList", PrimitiveTopology::PointList}, {"LineList", PrimitiveTopology::LineList},
                   {"LineStrip", PrimitiveTopology::LineStrip}, {"TriangleStrip", PrimitiveTopology::TriangleStrip});

    implement_enum(PolygonMode, Engine, {"Fill", PolygonMode::Fill}, {"Line", PolygonMode::Line}, {"Point", PolygonMode::Point});

    implement_enum(CullMode, Engine, {"None", CullMode::None}, {"Front", CullMode::Front}, {"Back", CullMode::Back});


    implement_enum(FrontFace, Engine, {"ClockWise", FrontFace::ClockWise}, {"CounterClockWise", FrontFace::CounterClockWise});

    implement_enum(WindowAttribute, Engine, {"None", WindowAttribute::None}, {"Resizable", WindowAttribute::Resizable},
                   {"FullScreen", WindowAttribute::FullScreen}, {"Shown", WindowAttribute::Shown},
                   {"Hidden", WindowAttribute::Hidden}, {"BorderLess", WindowAttribute::BorderLess},
                   {"MouseFocus", WindowAttribute::MouseFocus}, {"InputFocus", WindowAttribute::InputFocus},
                   {"InputGrabbed", WindowAttribute::InputGrabbed}, {"Minimized", WindowAttribute::Minimized},
                   {"Maximized", WindowAttribute::Maximized}, {"MouseCapture", WindowAttribute::MouseCapture},
                   {"MouseGrabbed", WindowAttribute::MouseGrabbed}, {"KeyboardGrabbed", WindowAttribute::KeyboardGrabbed});

    implement_enum(CursorMode, Engine, {"Normal", CursorMode::Normal}, {"Hidden", CursorMode::Hidden});

    implement_enum(Orientation, Engine, {"Landscape", Orientation::Landscape},
                   {"LandscapeFlipped", Orientation::LandscapeFlipped}, {"Portrait", Orientation::Portrait},
                   {"PortraitFlipped", Orientation::PortraitFlipped});

    implement_enum(MessageBoxType, Engine, {"Error", MessageBoxType::Error}, {"Warning", MessageBoxType::Warning},
                   {"Info", MessageBoxType::Info});

    implement_enum(VertexAttributeInputRate, Engine, {"Vertex", VertexAttributeInputRate::Vertex},
                   {"Instance", VertexAttributeInputRate::Instance});

    implement_enum(ColorComponentMask, Engine, {"RGBA", ColorComponentMask::RGBA}, {"RGB", ColorComponentMask::RGB},
                   {"RGA", ColorComponentMask::RGA}, {"RG", ColorComponentMask::RG}, {"RBA", ColorComponentMask::RBA},
                   {"RB", ColorComponentMask::RB}, {"RA", ColorComponentMask::RA}, {"R", ColorComponentMask::R},
                   {"GBA", ColorComponentMask::GBA}, {"GB", ColorComponentMask::GB}, {"GA", ColorComponentMask::GA},
                   {"G", ColorComponentMask::G}, {"BA", ColorComponentMask::BA}, {"B", ColorComponentMask::B},
                   {"A", ColorComponentMask::A});

    implement_enum(RenderPassType, Engine, {"Undefined", RenderPassType::Undefined}, {"Window", RenderPassType::Window},
                   {"SceneColor", RenderPassType::SceneColor}, {"GBuffer", RenderPassType::GBuffer});

    implement_enum(ViewMode, Engine, {"Lit", ViewMode::Lit}, {"Unlit", ViewMode::Unlit});

    implement_enum(VertexBufferElementType, Engine, {"Undefined", VertexBufferElementType::Undefined},
                   {"Float1", VertexBufferElementType::Float1}, {"Float2", VertexBufferElementType::Float2},
                   {"Float3", VertexBufferElementType::Float3}, {"Float4", VertexBufferElementType::Float4},
                   {"UByte4", VertexBufferElementType::UByte4}, {"UByte4N", VertexBufferElementType::UByte4N},
                   {"Color", VertexBufferElementType::Color});

    implement_enum(ColorFormat, Engine, {"Unknown", ColorFormat::Undefined}, {"FloatR", ColorFormat::FloatR},
                   {"FloatRGBA", ColorFormat::FloatRGBA}, {"R8", ColorFormat::R8}, {"R8G8B8A8", ColorFormat::R8G8B8A8},
                   {"DepthStencil", ColorFormat::DepthStencil}, {"ShadowDepth", ColorFormat::ShadowDepth},
                   {"FilteredShadowDepth", ColorFormat::FilteredShadowDepth}, {"D32F", ColorFormat::D32F},
                   {"BC1", ColorFormat::BC1}, {"BC2", ColorFormat::BC2}, {"BC3", ColorFormat::BC3});

    implement_enum(MaterialDomain, Engine, {"Surface", MaterialDomain::Surface});

    implement_enum(SplashTextType, Engine, {"StartupProgress", SplashTextType::StartupProgress},
                   {"VersionInfo", SplashTextType::VersionInfo}, {"CopyrightInfo", SplashTextType::CopyrightInfo},
                   {"GameName", SplashTextType::GameName});
}// namespace Engine
