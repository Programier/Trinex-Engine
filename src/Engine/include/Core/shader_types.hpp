#pragma once
#include <Core/buffer_types.hpp>
#include <Core/engine_types.hpp>
#include <Core/render_types.hpp>
#include <array>
#include <optional>


namespace Engine
{

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
        Zero                  = 0,
        One                   = 1,
        SrcColor              = 2,
        OneMinusSrcColor      = 3,
        DstColor              = 4,
        OneMinusDstColor      = 5,
        SrcAlpha              = 6,
        OneMinusSrcAlpha      = 7,
        DstAlpha              = 8,
        OneMinusDstAlpha      = 9,
        ConstantColor         = 10,
        OneMinusConstantColor = 11,
        ConstantAlpha         = 12,
        OneMinusConstantAlpha = 13,
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

    enum class EnableCap : EnumerateType
    {
        Blend       = 0,
        DepthTest   = 1,
        CullFace    = 2,
        StencilTest = 3,
    };


    using DepthFunc = CompareFunc;

    struct ShaderDataType {
        enum : EnumerateType
        {
            Bool  = 0,
            Int   = 1,
            UInt  = 2,
            Float = 3,
            Vec2  = 4,
            Vec3  = 5,
            Vec4  = 6,
            IVec2 = 7,
            IVec3 = 8,
            IVec4 = 9,
            UVec2 = 10,
            UVec3 = 11,
            UVec4 = 12,
            BVec2 = 13,
            BVec3 = 14,
            BVec4 = 15,
            Mat2  = 16,
            Mat3  = 17,
            Mat4  = 18
        } type;

        size_t size  = 0;
        size_t count = 1;

        template<class Type>
        inline static ShaderDataType type_of(size_t count = 1)
        {
            ShaderDataType result_type;
            result_type.size  = sizeof(Type) * count;
            result_type.count = count;

            static const Map<std::type_index, typeof(result_type.type)> types = {
                    {std::type_index(typeid(bool)), ShaderDataType::Bool},
                    {std::type_index(typeid(int_t)), ShaderDataType::Int},
                    {std::type_index(typeid(uint_t)), ShaderDataType::UInt},
                    {std::type_index(typeid(float)), ShaderDataType::Float},
                    {std::type_index(typeid(Vector2D)), ShaderDataType::Vec2},
                    {std::type_index(typeid(Vector3D)), ShaderDataType::Vec3},
                    {std::type_index(typeid(Vector4D)), ShaderDataType::Vec4},
                    {std::type_index(typeid(IntVector2D)), ShaderDataType::IVec2},
                    {std::type_index(typeid(IntVector3D)), ShaderDataType::IVec3},
                    {std::type_index(typeid(IntVector4D)), ShaderDataType::IVec4},
                    {std::type_index(typeid(UIntVector2D)), ShaderDataType::UVec2},
                    {std::type_index(typeid(UIntVector3D)), ShaderDataType::UVec3},
                    {std::type_index(typeid(UIntVector4D)), ShaderDataType::UVec4},
                    {std::type_index(typeid(BoolVector2D)), ShaderDataType::BVec2},
                    {std::type_index(typeid(BoolVector3D)), ShaderDataType::BVec3},
                    {std::type_index(typeid(BoolVector4D)), ShaderDataType::BVec4},
                    {std::type_index(typeid(Matrix2f)), ShaderDataType::Mat2},
                    {std::type_index(typeid(Matrix3f)), ShaderDataType::Mat3},
                    {std::type_index(typeid(Matrix4f)), ShaderDataType::Mat4},
            };

            result_type.type = types.at(std::type_index(typeid(Type)));
            return result_type;
        }
    };


    struct VertexAtribute {
        ShaderDataType type;
        ArrayIndex offset = 0;
    };

    struct VertexBufferInfo {
        Vector<VertexAtribute> attributes;
        size_t size;
        BindingIndex binding = 0;
    };

    struct ShaderTextureSampler {
        uint_t binding = 0;
    };

    struct ShaderUniformBuffer {
        String name;
        uint_t binding;
        size_t size;
    };

    struct ShaderShaderBuffer {
        uint_t binding = 0;
    };


    union BlendConstants
    {
        Array<float, 4> array = {0.0f, 0.0f, 0.0f, 0.0f};
        Vector4D vector;
    };

    enum class PrimitiveTopology : EnumerateType
    {
        TriangleList               = 0,
        PointList                  = 1,
        LineList                   = 2,
        LineStrip                  = 3,
        TriangleStrip              = 4,
        TriangleFan                = 5,
        LineListWithAdjacency      = 6,
        LineStripWithAdjacency     = 4,
        TriangleListWithAdjacency  = 8,
        TriangleStripWithAdjacency = 9,
        PatchList                  = 10,
    };

    enum class PolygonMode : EnumerateType
    {
        Fill  = 0,
        Line  = 1,
        Point = 2,
    };

    enum class CullMode : EnumerateType
    {
        None         = 0,
        Front        = 1,
        Back         = 2,
        FrontAndBack = 3,
    };

    enum class FrontFace : EnumerateType
    {
        ClockWise        = 0,
        CounterClockWise = 1,
    };

    enum class LogicOp : EnumerateType
    {
        Clear        = 0,
        And          = 1,
        AndReverse   = 2,
        Copy         = 3,
        AndInverted  = 4,
        NoOp         = 5,
        Xor          = 6,
        Or           = 7,
        Nor          = 8,
        Equivalent   = 10,
        Invert       = 11,
        OrReverse    = 12,
        CopyInverted = 13,
        OrInverted   = 14,
        Nand         = 15,
        Set          = 16,
    };


    using SampleMask         = size_t;
    using ColorComponentMask = size_t;

    struct ColorBlendAttachmentState {
        byte enable : 1               = 0;
        BlendFunc src_color_func      = BlendFunc::SrcColor;
        BlendFunc dst_color_func      = BlendFunc::OneMinusSrcColor;
        BlendOp color_op              = BlendOp::Add;
        BlendFunc src_alpha_func      = BlendFunc::SrcAlpha;
        BlendFunc dst_alpha_func      = BlendFunc::OneMinusSrcAlpha;
        BlendOp alpha_op              = BlendOp::Add;
        ColorComponentMask color_mask = ColorComponent::R | ColorComponent::G | ColorComponent::B | ColorComponent::A;
    };

    struct PipelineState {
        struct DepthTestInfo {
            byte enable : 1            = 1;
            byte write_enable : 1      = 1;
            byte bound_test_enable : 1 = 0;
            DepthFunc func             = DepthFunc::Less;
            float min_depth_bound      = 0.0;
            float max_depth_bound      = 0.0;
        } depth_test;

        struct StencilTestInfo {
            byte enable : 1 = 0;

            struct FaceInfo{
                StencilOp fail       = StencilOp::Decr;
                StencilOp depth_pass = StencilOp::Decr;
                StencilOp depth_fail = StencilOp::Decr;
                CompareFunc compare  = CompareFunc::Less;
                uint_t compare_mask  = 0;
                uint_t write_mask    = 0;
                int_t reference     = 0;
            } front, back;
        } stencil_test;

        struct AssemblyInfo {
            byte primitive_restart_enable : 1    = 0;
            PrimitiveTopology primitive_topology = PrimitiveTopology::TriangleList;
        } input_assembly;

        struct RasterizerInfo {
            byte depth_bias_enable : 1    = 0;
            byte discard_enable : 1       = 0;
            byte depth_clamp_enable : 1   = 0;
            float depth_bias_const_factor = 0.0;
            float depth_bias_clamp        = 0.0;
            float depth_bias_slope_factor = 0.0;
            PolygonMode poligon_mode      = PolygonMode::Fill;
            CullMode cull_mode            = CullMode::Front;
            FrontFace front_face          = FrontFace::CounterClockWise;
            float line_width              = 1.0;
        } rasterizer;

        struct ColorBlendingInfo {
            byte logic_op_enable : 1 = 0;
            LogicOp logic_op         = LogicOp::And;
            BlendConstants blend_constants;
            Vector<ColorBlendAttachmentState> blend_attachment;
        } color_blending;
    };


    struct PipelineCreateInfo {
        struct {
            FileBuffer vertex;
            FileBuffer fragment;
            FileBuffer compute;
            FileBuffer geometry;
        } binaries;

        struct {
            Vector<FileBuffer> vertex;
            Vector<FileBuffer> fragment;
            Vector<FileBuffer> compute;
            Vector<FileBuffer> geometry;
        } text;

        Vector<ShaderUniformBuffer> uniform_buffers;
        Vector<ShaderTextureSampler> texture_samplers;
        Vector<ShaderShaderBuffer> shared_buffers;

        std::string name;
        VertexBufferInfo vertex_info;
        Identifier framebuffer_usage               = 0;
        PipelineState* state                  = nullptr;
        uint_t max_textures_binding_per_frame = 100;
    };
}// namespace Engine