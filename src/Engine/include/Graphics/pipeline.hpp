#pragma once
#include <Core/pointer.hpp>
#include <Core/render_resource.hpp>
#include <Core/structures.hpp>

namespace Engine
{

    class VertexShader;
    class FragmentShader;
    class RenderPass;

    struct ENGINE_EXPORT LocalMaterialParametersInfo {
    public:
        using OffsetMap = Map<Name, size_t, Name::HashFunction>;

    private:
        OffsetMap _M_parameters_offset;

    public:
        static const size_t no_offset;

        bool empty() const;
        size_t size() const;

        const size_t offset_of(const Name& name) const;
        LocalMaterialParametersInfo& update(const Name& name, size_t new_offset);
        LocalMaterialParametersInfo& remove(const Name& name);
        const OffsetMap& offset_map() const;

        friend bool operator&(Archive& ar, LocalMaterialParametersInfo& info);
    };

    ENGINE_EXPORT bool operator&(Archive& ar, LocalMaterialParametersInfo& info);

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
            CullMode cull_mode       = CullMode::Front;
            FrontFace front_face     = FrontFace::CounterClockWise;
        } rasterizer;

        struct ALIGNED(4) ColorBlendAttachmentState {
            bool enable                   = false;
            BlendFunc src_color_func      = BlendFunc::SrcColor;
            BlendFunc dst_color_func      = BlendFunc::OneMinusSrcColor;
            BlendOp color_op              = BlendOp::Add;
            BlendFunc src_alpha_func      = BlendFunc::SrcAlpha;
            BlendFunc dst_alpha_func      = BlendFunc::OneMinusSrcAlpha;
            BlendOp alpha_op              = BlendOp::Add;
            ColorComponentMask color_mask = ColorComponentMask::RGBA;
        };

        struct ALIGNED(4) ColorBlendingInfo {
            Vector<ColorBlendAttachmentState> blend_attachment;
            LogicOp logic_op         = LogicOp::And;
            Vector4D blend_constants = {0.f, 0.f, 0.f, 0.f};
            bool logic_op_enable     = false;
        } ALIGNED(4) color_blending;


        LocalMaterialParametersInfo local_parameters;
        bool has_global_parameters;

    public:
        RenderPass* render_pass         = nullptr;
        VertexShader* vertex_shader     = nullptr;
        FragmentShader* fragment_shader = nullptr;

        Pipeline();
        ~Pipeline();
        Pipeline& rhi_create() override;
        Pipeline& postload() override;
        const Pipeline& rhi_bind() const;

        bool archive_process(class Archive& archive) override;
    };
}// namespace Engine
