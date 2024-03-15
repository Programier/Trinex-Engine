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
        struct ALIGNED(4) DepthTestInfo {
            DepthFunc func         = DepthFunc::Less;
            float min_depth_bound  = 0.0;
            float max_depth_bound  = 0.0;
            bool enable            = true;
            bool write_enable      = true;
            bool bound_test_enable = false;
        } depth_test;

        struct ALIGNED(4) StencilTestInfo {
            bool enable = false;

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

        struct ALIGNED(4) AssemblyInfo {
            PrimitiveTopology primitive_topology = PrimitiveTopology::TriangleList;
            bool primitive_restart_enable        = false;
        } input_assembly;

        struct ALIGNED(4) RasterizerInfo {
            float depth_bias_const_factor = 0.0;
            float depth_bias_clamp        = 0.0;
            float depth_bias_slope_factor = 0.0;
            float line_width              = 1.0;

            bool depth_bias_enable  = false;
            bool discard_enable     = false;
            bool depth_clamp_enable = false;

            PolygonMode polygon_mode = PolygonMode::Fill;
            CullMode cull_mode       = CullMode::Back;
            FrontFace front_face     = FrontFace::ClockWise;
        } rasterizer;

        struct ALIGNED(4) ColorBlendingInfo {
            bool enable                   = false;
            BlendFunc src_color_func      = BlendFunc::SrcColor;
            BlendFunc dst_color_func      = BlendFunc::OneMinusSrcColor;
            BlendOp color_op              = BlendOp::Add;
            BlendFunc src_alpha_func      = BlendFunc::SrcAlpha;
            BlendFunc dst_alpha_func      = BlendFunc::OneMinusSrcAlpha;
            BlendOp alpha_op              = BlendOp::Add;
            ColorComponentMask color_mask = ColorComponentMask::RGBA;


            LogicOp logic_op         = LogicOp::And;
            Vector4D blend_constants = {0.f, 0.f, 0.f, 0.f};
            bool logic_op_enable     = false;
        } ALIGNED(4) color_blending;


        MaterialScalarParametersInfo local_parameters;
        MaterialScalarParametersInfo global_parameters;

        Path shader_path;

        VertexShader* m_vertex_shader     = nullptr;
        FragmentShader* m_fragment_shader = nullptr;

    public:
        MaterialUsage usage = MaterialUsage::StaticMeshRendering;

        Pipeline();
        ~Pipeline();
        Pipeline& rhi_create() override;
        Pipeline& postload() override;
        const Pipeline& rhi_bind() const;
        class Material* material() const;
        RenderPassType render_pass_type() const;
        RenderPass* render_pass() const;
        VertexShader* vertex_shader() const;
        FragmentShader* fragment_shader() const;

        bool archive_process(class Archive& archive) override;
    };
}// namespace Engine
