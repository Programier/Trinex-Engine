#pragma once
#include <Core/pointer.hpp>
#include <Core/render_resource.hpp>
#include <Core/structures.hpp>

namespace Engine
{

    class VertexShader;
    class TessellationControlShader;
    class TessellationShader;
    class GeometryShader;
    class FragmentShader;
    class RenderPass;

    namespace ShaderCompiler
    {
        struct ShaderSource;
    }

    class ENGINE_EXPORT Pipeline : public RenderResource
    {
        declare_class(Pipeline, RenderResource);

    public:
        using ShadersArray = Array<class Shader*, 6>;

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
            BlendFunc src_color_func      = BlendFunc::SrcAlpha;
            BlendFunc dst_color_func      = BlendFunc::OneMinusSrcAlpha;
            BlendOp color_op              = BlendOp::Add;
            BlendFunc src_alpha_func      = BlendFunc::One;
            BlendFunc dst_alpha_func      = BlendFunc::Zero;
            BlendOp alpha_op              = BlendOp::Add;
            ColorComponentMask color_mask = ColorComponentMask::RGBA;


            LogicOp logic_op         = LogicOp::And;
            Vector4D blend_constants = {0.f, 0.f, 0.f, 0.f};
            bool logic_op_enable     = false;
        } ALIGNED(4) color_blending;

        TreeMap<Name, MaterialParameterInfo> parameters;
        MaterialScalarParametersInfo global_parameters;
        MaterialScalarParametersInfo local_parameters;
        Path shader_path;

    private:
        VertexShader* m_vertex_shader                            = nullptr;
        TessellationControlShader* m_tessellation_control_shader = nullptr;
        TessellationShader* m_tessellation_shader                = nullptr;
        GeometryShader* m_geometry_shader                        = nullptr;
        FragmentShader* m_fragment_shader                        = nullptr;

        template<typename Type>
        Type* create_new_shader(const char* name, Type*& out)
        {
            if (out)
                return out;

            out = Object::new_instance_named<Type>(name);
            out->flags(Object::IsAvailableForGC, false);
            out->owner(this);
            return out;
        }

    public:
        RenderPassType render_pass_type = RenderPassType::SceneColor;


        Pipeline();
        ~Pipeline();
        Pipeline& rhi_create() override;
        Pipeline& postload() override;
        const Pipeline& rhi_bind() const;
        class Material* material() const;
        RenderPass* render_pass() const;

        VertexShader* vertex_shader() const;
        FragmentShader* fragment_shader() const;
        TessellationControlShader* tessellation_control_shader() const;
        TessellationShader* tessellation_shader() const;
        GeometryShader* geometry_shader() const;

        VertexShader* vertex_shader(bool create = false);
        FragmentShader* fragment_shader(bool create = false);
        TessellationControlShader* tessellation_control_shader(bool create = false);
        TessellationShader* tessellation_shader(bool create = false);
        GeometryShader* geometry_shader(bool create = false);

        Pipeline& remove_vertex_shader();
        Pipeline& remove_fragment_shader();
        Pipeline& remove_tessellation_control_shader();
        Pipeline& remove_tessellation_shader();
        Pipeline& remove_geometry_shader();

        Flags<ShaderType> shader_type_flags() const;
        Pipeline& allocate_shaders(Flags<ShaderType> flags = 0);
        Pipeline& remove_shaders(Flags<ShaderType> flags = 0);
        const MaterialParameterInfo* find_param_info(const Name& name) const;
        bool submit_compiled_source(const ShaderCompiler::ShaderSource& source, MessageList& errors);
        size_t stages_count() const;

        FORCE_INLINE Pipeline& remove_all_shaders()
        {
            return remove_shaders(Flags<ShaderType>(~static_cast<BitMask>(0)));
        }

        ShadersArray shader_array() const;

        bool archive_process(class Archive& archive) override;
    };
}// namespace Engine
