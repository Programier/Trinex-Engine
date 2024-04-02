#include <Core/archive.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/enum.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Core/property.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/render_target_base.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader.hpp>

namespace Engine
{
    implement_struct(DepthTestInfo, Engine::Pipeline, ).push([]() {
        using DTI    = Pipeline::DepthTestInfo;
        Struct* self = Struct::static_find("Engine::Pipeline::DepthTestInfo", true);

        self->add_properties(
                new EnumProperty("Func", "Depth compare function", &DTI::func, Enum::find("Engine::DepthFunc", true)),
                new FloatProperty("Min depth bound", "Min depth bound", &DTI::min_depth_bound),
                new FloatProperty("Max depth bound", "Max depth bound", &DTI::max_depth_bound),
                new BoolProperty("Enable", "Enable depth test", &DTI::enable),
                new BoolProperty("Write Enable", "Enable write to depth buffer", &DTI::write_enable),
                new BoolProperty("Enable bound test", "Enable bound test", &DTI::bound_test_enable));
    });

    implement_struct(FaceInfo, Engine::Pipeline::StencilTestInfo, ).push([]() {
        using FI                = Pipeline::StencilTestInfo::FaceInfo;
        Enum* stencil_op_enum   = Enum::find("Engine::StencilOp", true);
        Enum* compare_func_enum = Enum::find("Engine::CompareFunc", true);
        Struct* self            = Struct::static_find("Engine::Pipeline::StencilTestInfo::FaceInfo", true);

        self->add_properties(new EnumProperty("Fail", "Operation on fail", &FI::fail, stencil_op_enum),
                             new EnumProperty("Depth pass", "Operation on depth pass", &FI::depth_pass, stencil_op_enum),
                             new EnumProperty("Depth Fail", "Operation on depth fail", &FI::depth_fail, stencil_op_enum),
                             new EnumProperty("Compare func", "Stencil compare function", &FI::compare, compare_func_enum),
                             new UIntProperty("Compare mask", "Stencil compare mask", &FI::compare_mask),
                             new UIntProperty("Write mask", "Stencil write mask", &FI::write_mask),
                             new IntProperty("Reference", "Stencil reference", &FI::reference));
    });

    implement_struct(StencilTestInfo, Engine::Pipeline, ).push([]() {
        using STI = Pipeline::StencilTestInfo;

        Struct* self      = Struct::static_find("Engine::Pipeline::StencilTestInfo", true);
        Struct* face_info = Struct::static_find("Engine::Pipeline::StencilTestInfo::FaceInfo", true);

        self->add_properties(new BoolProperty("Enable", "Enable stencil test", &STI::enable),
                             new StructProperty("Front", "Stencil parameters for front face", &STI::front, face_info),
                             new StructProperty("Back", "Stencil parameters for back face", &STI::back, face_info));
    });

    implement_struct(AssemblyInfo, Engine::Pipeline, ).push([]() {
        using AI = Pipeline::AssemblyInfo;

        Struct* self = Struct::static_find("Engine::Pipeline::AssemblyInfo", true);

        self->add_properties(new EnumProperty("Primitive Topology", "Primitive types which will be rendered by this pipeline",
                                              &AI::primitive_topology, Enum::find("Engine::PrimitiveTopology", true)),
                             new BoolProperty("Enable restart", "Enable primitive restart", &AI::primitive_restart_enable));
    });

    implement_struct(RasterizerInfo, Engine::Pipeline, ).push([]() {
        using RI = Pipeline::RasterizerInfo;

        Struct* self = Struct::static_find("Engine::Pipeline::RasterizerInfo", true);

        self->add_properties(
                new FloatProperty("Depth bias const factor", "Depth bias const factor", &RI::depth_bias_const_factor),
                new FloatProperty("Depth bias clamp", "Depth bias clamp", &RI::depth_bias_clamp),
                new FloatProperty("Depth bias slope factor", "Depth bias slope factor", &RI::depth_bias_slope_factor),
                new FloatProperty("Line width", "Width of line which will be rendered by this material", &RI::line_width),

                new BoolProperty("Enable depth bias", "Enable depth bias", &RI::depth_bias_enable),
                new BoolProperty("Enable discard", "If true then shaders can use discard keyword", &RI::discard_enable),
                new BoolProperty("Enable depth clamp", "Enable depth clamp", &RI::depth_clamp_enable),

                new EnumProperty("Polygon mode", "Polygon Mode", &RI::polygon_mode, Enum::find("Engine::PolygonMode", true)),
                new EnumProperty("Cull mode", "Cull Mode", &RI::cull_mode, Enum::find("Engine::CullMode", true)),
                new EnumProperty("Front face", "Front face", &RI::front_face, Enum::find("Engine::FrontFace", true)));
    });

    implement_struct(ColorBlendingInfo, Engine::Pipeline, ).push([]() {
        using CBI    = Pipeline::ColorBlendingInfo;
        Struct* self = Struct::static_find("Engine::Pipeline::ColorBlendingInfo", true);

        Enum* blend_func = Enum::find("Engine::BlendFunc", true);
        Enum* blend_op   = Enum::find("Engine::BlendOp", true);

        self->add_properties(
                new BoolProperty("Enable", "Enable blending", &CBI::enable),
                new EnumProperty("Src color func", "Src color func", &CBI::src_color_func, blend_func),
                new EnumProperty("Dst color func", "Dst color func", &CBI::dst_color_func, blend_func),
                new EnumProperty("Color operator", "Color operator", &CBI::color_op, blend_op),

                new EnumProperty("Src alpha func", "Src alpha func", &CBI::src_alpha_func, blend_func),
                new EnumProperty("Dst alpha func", "Dst alpha func", &CBI::dst_alpha_func, blend_func),
                new EnumProperty("Alpha operator", "Alpha operator", &CBI::alpha_op, blend_op),
                new EnumProperty("Color mask", "Color mask", &CBI::color_mask, Enum::find("Engine::ColorComponentMask", true)));

        self->add_properties(
                new EnumProperty("Logic operator", "Logic operator", &CBI::logic_op, Enum::find("Engine::LogicOp", true)),
                new Vec4Property("Blend constants", "Blend constant values", &CBI::blend_constants),
                new BoolProperty("Enable logic operator", "Enable logic operator", &CBI::logic_op_enable));
    });


    Pipeline::Pipeline()
    {}

    Pipeline::~Pipeline()
    {
        remove_all_shaders();
    }

#define init_shader(sdr)                                                                                                         \
    if (sdr)                                                                                                                     \
    sdr->rhi_create()

    Pipeline& Pipeline::rhi_create()
    {
        init_shader(m_vertex_shader);
        init_shader(m_tessellation_control_shader);
        init_shader(m_tessellation_shader);
        init_shader(m_geometry_shader);
        init_shader(m_fragment_shader);
        m_rhi_object.reset(engine_instance->rhi()->create_pipeline(this));
        return *this;
    }

    Pipeline& Pipeline::postload()
    {
        // Initialize shaders first!
        Super::postload();

        return *this;
    }

    const Pipeline& Pipeline::rhi_bind() const
    {
        if (m_rhi_object)
        {
            rhi_object<RHI_Pipeline>()->bind();
        }
        return *this;
    }

    class Material* Pipeline::material() const
    {
        return Object::instance_cast<Material>(owner());
    }

    RenderPassType Pipeline::render_pass_type() const
    {
        Flags<MaterialUsage> flags = usage;

        if ((flags & MaterialUsage::GBufferRendering) == Flags(MaterialUsage::GBufferRendering) && !color_blending.enable)
        {
            return RenderPassType::GBuffer;
        }

        if ((flags & MaterialUsage::WindowRendering) == Flags(MaterialUsage::WindowRendering))
        {
            return RenderPassType::Window;
        }

        if ((flags & MaterialUsage::SceneOutputRendering) == Flags(MaterialUsage::SceneOutputRendering) || color_blending.enable)
        {
            return RenderPassType::OneAttachentOutput;
        }

        return RenderPassType::GBuffer;
    }

    RenderPass* Pipeline::render_pass() const
    {
        return RenderPass::load_render_pass(render_pass_type());
    }

    VertexShader* Pipeline::vertex_shader() const
    {
        return m_vertex_shader;
    }

    FragmentShader* Pipeline::fragment_shader() const
    {
        return m_fragment_shader;
    }

    TessellationControlShader* Pipeline::tessellation_control_shader() const
    {
        return m_tessellation_control_shader;
    }

    TessellationShader* Pipeline::tessellation_shader() const
    {
        return m_tessellation_shader;
    }

    GeometryShader* Pipeline::geometry_shader() const
    {
        return m_geometry_shader;
    }


    VertexShader* Pipeline::vertex_shader(bool create)
    {
        if (!m_vertex_shader && create)
        {
            allocate_shaders(ShaderType::Vertex);
        }

        return m_vertex_shader;
    }

    FragmentShader* Pipeline::fragment_shader(bool create)
    {
        if (!m_fragment_shader && create)
        {
            allocate_shaders(ShaderType::Fragment);
        }

        return m_fragment_shader;
    }

    TessellationControlShader* Pipeline::tessellation_control_shader(bool create)
    {
        if (!m_tessellation_control_shader && create)
        {
            allocate_shaders(ShaderType::TessellationControl);
        }

        return m_tessellation_control_shader;
    }

    TessellationShader* Pipeline::tessellation_shader(bool create)
    {
        if (!m_tessellation_shader && create)
        {
            allocate_shaders(ShaderType::Tessellation);
        }

        return m_tessellation_shader;
    }

    GeometryShader* Pipeline::geometry_shader(bool create)
    {
        if (!m_geometry_shader && create)
        {
            allocate_shaders(ShaderType::Geometry);
        }

        return m_geometry_shader;
    }

    Pipeline& Pipeline::remove_vertex_shader()
    {
        if (m_vertex_shader)
        {
            delete m_vertex_shader;
            m_vertex_shader = nullptr;
        }
        return *this;
    }

    Pipeline& Pipeline::remove_fragment_shader()
    {
        if (m_fragment_shader)
        {
            delete m_fragment_shader;
            m_fragment_shader = nullptr;
        }
        return *this;
    }

    Pipeline& Pipeline::remove_tessellation_control_shader()
    {
        if (m_tessellation_control_shader)
        {
            delete m_tessellation_control_shader;
            m_tessellation_control_shader = nullptr;
        }
        return *this;
    }

    Pipeline& Pipeline::remove_tessellation_shader()
    {
        if (m_tessellation_shader)
        {
            delete m_tessellation_shader;
            m_tessellation_shader = nullptr;
        }
        return *this;
    }

    Pipeline& Pipeline::remove_geometry_shader()
    {
        if (m_geometry_shader)
        {
            delete m_geometry_shader;
            m_geometry_shader = nullptr;
        }
        return *this;
    }

    Flags<ShaderType> Pipeline::shader_type_flags() const
    {
        Flags<ShaderType> result = Flags(ShaderType::Vertex) | Flags(ShaderType::Fragment);

        if (m_tessellation_control_shader)
            result(ShaderType::TessellationControl, true);
        if (m_tessellation_shader)
            result(ShaderType::Tessellation, true);
        if (m_geometry_shader)
            result(ShaderType::Geometry, true);

        return result;
    }

    Pipeline& Pipeline::allocate_shaders(Flags<ShaderType> flags)
    {
        if (flags & ShaderType::Vertex)
        {
            create_new_shader<VertexShader>("Vertex Shader", m_vertex_shader);
        }

        if (flags & ShaderType::Fragment)
        {
            create_new_shader<FragmentShader>("Fragment Shader", m_fragment_shader);
        }

        if (flags & ShaderType::TessellationControl)
        {
            create_new_shader<TessellationControlShader>("Tessellation Control Shader", m_tessellation_control_shader);
        }

        if (flags & ShaderType::Tessellation)
        {
            create_new_shader<TessellationShader>("Tessellation Shader", m_tessellation_shader);
        }

        if (flags & ShaderType::Geometry)
        {
            create_new_shader<GeometryShader>("Geometry Shader", m_geometry_shader);
        }

        return *this;
    }

    Pipeline& Pipeline::remove_shaders(Flags<ShaderType> flags)
    {
        if (flags & ShaderType::Vertex)
        {
            remove_vertex_shader();
        }

        if (flags & ShaderType::Fragment)
        {
            remove_fragment_shader();
        }

        if (flags & ShaderType::TessellationControl)
        {
            remove_tessellation_control_shader();
        }

        if (flags & ShaderType::Tessellation)
        {
            remove_tessellation_shader();
        }

        if (flags & ShaderType::Geometry)
        {
            remove_geometry_shader();
        }
        return *this;
    }

    Pipeline::ShadersArray Pipeline::shader_array() const
    {
        return {
                m_vertex_shader,              //
                m_tessellation_control_shader,//
                m_tessellation_shader,        //
                m_geometry_shader,            //
                m_fragment_shader,            //
                nullptr                       //
        };
    }

    static bool serialize_shader_sources(const Path& path, Pipeline* pipeline, bool is_reading)
    {
        union
        {
            BufferReader* reader = nullptr;
            BufferWriter* writer;
        };

        bool status = true;

        Archive archive;

        if (is_reading)
        {
            reader = new FileReader(path);

            if (!reader->is_open())
            {
                delete reader;
                return false;
            }

            archive = reader;
        }
        else
        {
            rootfs()->create_dir(path.base_path());
            writer = new FileWriter(path);

            if (!writer->is_open())
            {
                delete writer;
                return false;
            }

            archive = writer;
        }

        for (Shader* shader : pipeline->shader_array())
        {
            if (shader)
            {
                status = status && shader->archive_process_source_code(archive);
            }
        }

        if (is_reading)
        {
            delete reader;
        }
        else
        {
            delete writer;
        }


        return status;
    }

    bool Pipeline::archive_process(class Archive& archive)
    {
        static auto serialize_shader_internal = [](Shader* shader, Archive& archive) {
            if (shader)
            {
                shader->archive_process(archive);
            }
        };

        Material* material_object = material();
        if (material_object == nullptr)
        {
            error_log("Pipeline", "Cannot serialize pipeline! Pipeline must be child of material!");
            return false;
        }

        if (!Super::archive_process(archive))
            return false;

        auto flags = shader_type_flags();

        archive & flags;
        archive & global_parameters;
        archive & local_parameters;

        allocate_shaders(flags);

        serialize_shader_internal(m_vertex_shader, archive);
        serialize_shader_internal(m_tessellation_control_shader, archive);
        serialize_shader_internal(m_tessellation_shader, archive);
        serialize_shader_internal(m_geometry_shader, archive);
        serialize_shader_internal(m_fragment_shader, archive);

        // Loading shaders from shader cache
        String material_name = material_object->full_name(true);

        Path path = Strings::format(
                "{}{}{}{}{}{}", engine_config.shader_cache_dir.str(), Path::separator, engine_config.api, Path::separator,
                Strings::replace_all(material_name, Constants::name_separator, Path::sv_separator), Constants::shader_extention);


        bool status = serialize_shader_sources(path, this, archive.is_reading());

        if (!status && archive.is_reading())
        {
            warn_log("Pipeline", "Missing shader cache for material '%s'. Recompiling...", material_name.c_str());

            if ((status = material_object->compile()))
            {
                info_log("Pipeline", "Compile success!");

                // If compile is success, serialize source to file
                serialize_shader_sources(path, this, false);
            }
            else
            {
                error_log("Pipeline", "Compile fail!");
            }
        }

        return archive && status;
    }

    implement_class(Pipeline, Engine, 0);
    implement_initialize_class(Pipeline)
    {
        Class* self = static_class_instance();

        Enum* material_usage_enum = Enum::find("Engine::MaterialUsage", true);

        auto render_pass_prop =
                new EnumProperty("Usage", "Type of usage of this pipeline", &Pipeline::usage, material_usage_enum);

        auto path_prop = new PathProperty("Shader Path", "Path to slang file", &This::shader_path);

        path_prop->on_prop_changed.push([](void* object) {
            Pipeline* pipeline    = reinterpret_cast<Pipeline*>(object);
            pipeline->shader_path = pipeline->shader_path.relative(engine_config.shaders_dir);
        });

        self->add_properties(new StructProperty("Depth Test", "Depth Test properties", &Pipeline::depth_test,
                                                Struct::static_find("Engine::Pipeline::DepthTestInfo", true)),
                             new StructProperty("Stencil Test", "Stencil Test properties", &Pipeline::stencil_test,
                                                Struct::static_find("Engine::Pipeline::StencilTestInfo", true)),
                             new StructProperty("Assembly Input", "Assembly Input", &Pipeline::input_assembly,
                                                Struct::static_find("Engine::Pipeline::AssemblyInfo", true)),
                             new StructProperty("Rasterizer", "Rasterizer properties", &Pipeline::rasterizer,
                                                Struct::static_find("Engine::Pipeline::RasterizerInfo", true)),
                             new StructProperty("Color blending", "Blending properties", &Pipeline::color_blending,
                                                Struct::static_find("Engine::Pipeline::ColorBlendingInfo", true)),
                             render_pass_prop, path_prop);
    }
}// namespace Engine
