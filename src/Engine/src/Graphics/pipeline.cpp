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
    ENGINE_EXPORT bool operator&(Archive& ar, MaterialScalarParametersInfo& info)
    {
        ar & info.m_binding_index;
        return ar;
    }

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
    {
        vertex_shader = Object::new_instance_named<VertexShader>("Vertex Shader");
        vertex_shader->flags(Object::IsAvailableForGC, false);
        vertex_shader->owner(this);

        fragment_shader = Object::new_instance_named<FragmentShader>("Fragment Shader");
        fragment_shader->flags(Object::IsAvailableForGC, false);
        fragment_shader->owner(this);
    }

    Pipeline::~Pipeline()
    {
        delete vertex_shader;
        delete fragment_shader;
    }

    Pipeline& Pipeline::rhi_create()
    {
        vertex_shader->rhi_create();
        fragment_shader->rhi_create();
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

    bool Pipeline::serialize_shaders(Archive& ar)
    {
        if (ar.is_open())
        {
            vertex_shader->archive_process(ar);
            fragment_shader->archive_process(ar);
        }

        return static_cast<bool>(ar);
    }

    bool Pipeline::archive_process(class Archive& archive)
    {
        Material* material_object = material();
        if (material_object == nullptr)
        {
            error_log("Pipeline", "Cannot serialize pipeline! Pipeline must be child of material!");
            return false;
        }

        if (!Super::archive_process(archive))
            return false;

        archive & global_parameters;
        archive & local_parameters;


        String api_name = engine_config.api;
        archive & api_name;

        size_t shader_code_size_start = archive.position();

        size_t shader_code_size = 0;
        archive & shader_code_size;

        bool status = false;

        size_t shader_code_start = archive.position();

        if (archive.is_saving())
        {
            serialize_shaders(archive);

            auto end_position = archive.position();
            shader_code_size  = end_position - shader_code_start;

            archive.position(shader_code_size_start);
            archive & shader_code_size;
            archive.position(end_position);
        }
        else if (archive.is_reading())
        {
            if (api_name == engine_config.api)
            {
                serialize_shaders(archive);
            }
            else
            {
                archive.position(shader_code_start + shader_code_size);
            }
        }

        if (archive.is_saving() || api_name != engine_config.api)
        {
            // Loading shaders from shader cache
            Path path = Strings::format(
                    "{}{}{}{}{}{}", engine_config.shader_cache_dir.str(), Path::separator, engine_config.api, Path::separator,
                    Strings::replace_all(material_object->full_name(true), Constants::name_separator, Path::sv_separator),
                    Constants::shader_extention);

            union
            {
                BufferReader* reader = nullptr;
                BufferWriter* writer;
            };

            Archive second_archive;

            if (archive.is_reading())
            {
                reader         = new FileReader(path);
                second_archive = reader;
            }
            else if (archive.is_saving())
            {
                rootfs()->create_dir(path.base_path());
                writer         = new FileWriter(path);
                second_archive = writer;
            }

            serialize_shaders(second_archive);

            if (second_archive.is_reading())
            {
                delete reader;
            }
            else
            {
                delete writer;
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
