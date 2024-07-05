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
            DepthFunc func    = DepthFunc::Less;
            bool enable       = true;
            bool write_enable = true;
        } depth_test;

        struct ALIGNED(4) StencilTestInfo {
            bool enable = false;

            StencilOp fail       = StencilOp::Decr;
            StencilOp depth_pass = StencilOp::Decr;
            StencilOp depth_fail = StencilOp::Decr;
            CompareFunc compare  = CompareFunc::Less;
            byte compare_mask    = 0;
            byte write_mask      = 0;
        } stencil_test;

        struct ALIGNED(4) AssemblyInfo {
            PrimitiveTopology primitive_topology = PrimitiveTopology::TriangleList;
        } input_assembly;

        struct ALIGNED(4) RasterizerInfo {
            PolygonMode polygon_mode = PolygonMode::Fill;
            CullMode cull_mode       = CullMode::Back;
            FrontFace front_face     = FrontFace::ClockWise;
            float line_width         = 1.f;
        } rasterizer;

        struct ALIGNED(4) ColorBlendingInfo {
            bool enable          = false;


            BlendFunc src_color_func      = BlendFunc::SrcAlpha;
            BlendFunc dst_color_func      = BlendFunc::OneMinusSrcAlpha;
            BlendOp color_op              = BlendOp::Add;
            BlendFunc src_alpha_func      = BlendFunc::One;
            BlendFunc dst_alpha_func      = BlendFunc::Zero;
            BlendOp alpha_op              = BlendOp::Add;
            ColorComponentMask color_mask = ColorComponentMask::RGBA;
        } ALIGNED(4) color_blending;

        TreeMap<Name, MaterialParameterInfo> parameters;
        MaterialScalarParametersInfo global_parameters;
        MaterialScalarParametersInfo local_parameters;

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
        Pipeline();
        ~Pipeline();
        Pipeline& rhi_create() override;
        Pipeline& postload() override;
        const Pipeline& rhi_bind() const;
        class Material* material() const;

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
