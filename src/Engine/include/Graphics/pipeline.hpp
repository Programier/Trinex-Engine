#pragma once
#include <Core/pointer.hpp>
#include <Core/render_resource.hpp>
#include <Core/structures.hpp>

namespace Engine
{

    class VertexShader;
    class FragmentShader;
    class RenderPass;

    class ENGINE_EXPORT Pipeline : public RenderResource
    {
        declare_class(Pipeline, RenderResource);

    public:
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

        struct ColorBlendAttachmentState {
            byte enable : 1               = 0;
            BlendFunc src_color_func      = BlendFunc::SrcColor;
            BlendFunc dst_color_func      = BlendFunc::OneMinusSrcColor;
            BlendOp color_op              = BlendOp::Add;
            BlendFunc src_alpha_func      = BlendFunc::SrcAlpha;
            BlendFunc dst_alpha_func      = BlendFunc::OneMinusSrcAlpha;
            BlendOp alpha_op              = BlendOp::Add;
            ColorComponentMask color_mask = mask_of<ColorComponentMask>(ColorComponent::R, ColorComponent::G,
                                                                        ColorComponent::B, ColorComponent::A);
        };

        struct ColorBlendingInfo {
            Vector<ColorBlendAttachmentState> blend_attachment;
            LogicOp logic_op = LogicOp::And;
            BlendConstants blend_constants;

            byte logic_op_enable = 0;
        } color_blending;

    public:
        Pointer<RenderPass> render_pass;
        Pointer<VertexShader> vertex_shader;
        Pointer<FragmentShader> fragment_shader;

        Pipeline& rhi_create() override;
        const Pipeline& rhi_bind() const;
    };
}// namespace Engine
