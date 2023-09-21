#pragma once
#include <Core/structures.hpp>
#include <optional>

namespace Engine
{

    namespace RHI
    {
        struct RHI_FrameBuffer;
        struct RHI_Texture;
    }// namespace RHI

    struct FrameBufferAttachment {
        RHI::RHI_Texture* texture = nullptr;
        MipMapLevel mip_level     = 0;
    };

    union FrameBufferClearValue
    {
        ColorClearValue color;
        DepthStencilClearValue depth_stencil;

        FrameBufferClearValue() : color(0.0f, 0.0f, 0.0f, 1.0f)
        {}
    };

    struct FrameBufferAttachmentClearData {
        byte clear_on_bind : 1 = 1;
        FrameBufferClearValue clear_value;
    };

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
        static ShaderDataType type_of(size_t count = 1)
        {

            ShaderDataType result_type;
            result_type.size  = sizeof(Type) * count;
            result_type.count = count;


            private_type_of(result_type, std::type_index(typeid(Type)));
            return result_type;
        }

    private:
        static ENGINE_EXPORT void private_type_of(ShaderDataType& result, const std::type_index& index);
    };

    struct VertexAttribute {
        String name;
        ShaderDataType type;
        ArrayIndex offset = 0;

        inline VertexAttribute(const String& _name = "") : name(_name)
        {}
    };

    struct VertexBufferInfo {
        Vector<VertexAttribute> attributes;
        size_t size;
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


    struct ColorBlendAttachmentState {
        byte enable : 1          = 0;
        BlendFunc src_color_func = BlendFunc::SrcColor;
        BlendFunc dst_color_func = BlendFunc::OneMinusSrcColor;
        BlendOp color_op         = BlendOp::Add;
        BlendFunc src_alpha_func = BlendFunc::SrcAlpha;
        BlendFunc dst_alpha_func = BlendFunc::OneMinusSrcAlpha;
        BlendOp alpha_op         = BlendOp::Add;
        ColorComponentMask color_mask =
                mask_of<ColorComponentMask>(ColorComponent::R, ColorComponent::G, ColorComponent::B, ColorComponent::A);
    };

    struct PipelineState {
        struct DepthTestInfo {
            DepthFunc func             = DepthFunc::Less;
            float min_depth_bound      = 0.0;
            float max_depth_bound      = 0.0;
            byte enable : 1            = 1;
            byte write_enable : 1      = 1;
            byte bound_test_enable : 1 = 0;
        } depth_test;

        struct StencilTestInfo {
            byte enable : 1 = 0;

            struct FaceInfo {
                StencilOp fail       = StencilOp::Decr;
                StencilOp depth_pass = StencilOp::Decr;
                StencilOp depth_fail = StencilOp::Decr;
                CompareFunc compare  = CompareFunc::Less;
                uint_t compare_mask  = 0;
                uint_t write_mask    = 0;
                int_t reference      = 0;
            } front, back;
        } stencil_test;

        struct AssemblyInfo {
            PrimitiveTopology primitive_topology = PrimitiveTopology::TriangleList;
            byte primitive_restart_enable : 1    = 0;
        } input_assembly;

        struct RasterizerInfo {
            float depth_bias_const_factor = 0.0;
            float depth_bias_clamp        = 0.0;
            float depth_bias_slope_factor = 0.0;
            float line_width              = 1.0;

            byte depth_bias_enable : 1  = 0;
            byte discard_enable : 1     = 0;
            byte depth_clamp_enable : 1 = 0;
            PolygonMode poligon_mode    = PolygonMode::Fill;
            CullMode cull_mode          = CullMode::Front;
            FrontFace front_face        = FrontFace::CounterClockWise;
        } rasterizer;

        struct ColorBlendingInfo {
            Vector<ColorBlendAttachmentState> blend_attachment;
            LogicOp logic_op = LogicOp::And;
            BlendConstants blend_constants;

            byte logic_op_enable = 0;
        } color_blending;
    };


    ////////////////////////////////////////////////////////////////////////////////

    struct FrameBufferCreateInfo {
        struct Buffer {
            Vector<FrameBufferAttachment> color_attachments;
            std::optional<FrameBufferAttachment> depth_stencil_attachment;
        };

        Vector<Buffer> buffers;
        Vector<FrameBufferAttachmentClearData> color_clear_data;
        FrameBufferAttachmentClearData depth_stencil_clear_data;
        Size2D size;
    };

    struct PipelineCreateInfo {
        PipelineState state;

        struct {
            FileBuffer vertex;
            FileBuffer fragment;
            FileBuffer compute;
            FileBuffer geometry;
        } binaries;

        struct {
            FileBuffer vertex;
            FileBuffer fragment;
            FileBuffer compute;
            FileBuffer geometry;
        } text;

        Vector<ShaderUniformBuffer> uniform_buffers;
        Vector<ShaderTextureSampler> texture_samplers;
        Vector<ShaderShaderBuffer> shared_buffers;

        String name;
        VertexBufferInfo vertex_info;
        RHI::RHI_FrameBuffer* framebuffer;
    };

    struct SamplerCreateInfo {
        TextureFilter min_filter      = TextureFilter::Linear;
        TextureFilter mag_filter      = TextureFilter::Linear;
        SamplerMipmapMode mipmap_mode = SamplerMipmapMode::Linear;
        WrapValue wrap_s              = WrapValue::Repeat;
        WrapValue wrap_t              = WrapValue::Repeat;
        WrapValue wrap_r              = WrapValue::Repeat;
        float mip_lod_bias            = 0.0;
        float anisotropy              = 1.0;
        CompareMode compare_mode      = CompareMode::None;
        float min_lod                 = -1000.0;
        float max_lod                 = 1000.0;
        CompareFunc compare_func      = CompareFunc::Always;
        bool unnormalized_coordinates;
    };

    struct TextureCreateInfo {
        Size2D size                   = {1, 1};
        MipMapLevel base_mip_level    = 0;
        MipMapLevel mipmap_count      = 1;
        ColorFormat format            = ColorFormat::R8G8B8A8Unorm;
        SwizzleRGBA swizzle;
    };
}// namespace Engine
