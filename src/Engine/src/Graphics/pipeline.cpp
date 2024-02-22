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
    const size_t LocalMaterialParametersInfo::no_offset = ~static_cast<size_t>(0);

    const size_t LocalMaterialParametersInfo::offset_of(const Name& name) const
    {
        auto it = m_parameters_offset.find(name);
        if (it != m_parameters_offset.end())
            return it->second;

        return no_offset;
    }

    LocalMaterialParametersInfo& LocalMaterialParametersInfo::update(const Name& name, size_t new_offset)
    {
        if (name.is_valid())
        {
            m_parameters_offset[name] = new_offset;
        }
        return *this;
    }

    LocalMaterialParametersInfo& LocalMaterialParametersInfo::remove(const Name& name)
    {
        m_parameters_offset.erase(name);
        return *this;
    }

    const LocalMaterialParametersInfo::OffsetMap& LocalMaterialParametersInfo::offset_map() const
    {
        return m_parameters_offset;
    }

    bool LocalMaterialParametersInfo::empty() const
    {
        return m_parameters_offset.empty();
    }

    size_t LocalMaterialParametersInfo::size() const
    {
        return m_parameters_offset.size();
    }

    ENGINE_EXPORT bool operator&(Archive& ar, LocalMaterialParametersInfo& info)
    {
        size_t count = info.m_parameters_offset.size();
        ar & count;

        if (ar.is_saving())
        {
            Name name;
            for (auto& ell : info.m_parameters_offset)
            {
                name = ell.first;
                ar & name;
                ar & ell.second;
            }
        }
        else
        {
            Name name;
            size_t offset = 0;
            while (count > 0)
            {
                ar & name;
                ar & offset;

                info.update(name, offset);
                --count;
            }
        }

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


    implement_struct(ColorBlendAttachmentState, Engine::Pipeline, ).push([]() {
        using CBAS = Pipeline::ColorBlendAttachmentState;

        Struct* self     = Struct::static_find("Engine::Pipeline::ColorBlendAttachmentState", true);
        Enum* blend_func = Enum::find("Engine::BlendFunc", true);
        Enum* blend_op   = Enum::find("Engine::BlendOp", true);

        self->add_properties(
                new BoolProperty("Enable", "Enable blending", &CBAS::enable),
                new EnumProperty("Src color func", "Src color func", &CBAS::src_color_func, blend_func),
                new EnumProperty("Dst color func", "Dst color func", &CBAS::dst_color_func, blend_func),
                new EnumProperty("Color operator", "Color operator", &CBAS::color_op, blend_op),

                new EnumProperty("Src alpha func", "Src alpha func", &CBAS::src_alpha_func, blend_func),
                new EnumProperty("Dst alpha func", "Dst alpha func", &CBAS::dst_alpha_func, blend_func),
                new EnumProperty("Alpha operator", "Alpha operator", &CBAS::alpha_op, blend_op),

                new EnumProperty("Color mask", "Color mask", &CBAS::color_mask, Enum::find("Engine::ColorComponentMask", true)));
    });

    implement_struct(ColorBlendingInfo, Engine::Pipeline, ).push([]() {
        using CBI = Pipeline::ColorBlendingInfo;

        Struct* self = Struct::static_find("Engine::Pipeline::ColorBlendingInfo", true);

        Struct* attachment_state = Struct::static_find("Engine::Pipeline::ColorBlendAttachmentState", true);

        self->add_properties(
                new ArrayProperty(
                        "Blend attachments", "Blend attachments", &CBI::blend_attachment,
                        new StructProperty<Object, Pipeline::ColorBlendAttachmentState>("", "", nullptr, attachment_state)),
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


    bool Pipeline::serialize_shaders(Archive& ar)
    {
        if (ar.is_open())
        {
            ar & local_parameters;
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

        archive & has_global_parameters;

        String shader_lang = engine_config.shading_language;
        archive & shader_lang;

        size_t shader_code_size_start = archive.position();

        size_t shader_code_size = 0;
        archive & shader_code_size;

        bool status = false;

        size_t shader_code_start = archive.position();

        if (archive.is_saving())
        {
            serialize_shaders(archive);

            auto end_position = archive.position();
            shader_code_size = end_position - shader_code_start;

            archive.position(shader_code_size_start);
            archive & shader_code_size;
            archive.position(end_position);
        }
        else if (archive.is_reading())
        {
            if (shader_lang == engine_config.shading_language)
            {
                serialize_shaders(archive);
            }
            else
            {
                archive.position(shader_code_start + shader_code_size);
            }
        }

        if (archive.is_saving() || shader_lang != engine_config.shading_language)
        {
            // Loading shaders from shader cache
            Path path = Strings::format(
                    "ShaderCache{}{}{}", Path::separator,
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

        Enum* render_pass_type_enum = Enum::find("Engine::RenderPassType", true);

        auto render_pass_prop = new EnumProperty("Render pass", "Type of render pass for this pipeline", &Pipeline::render_pass,
                                                 render_pass_type_enum);

        render_pass_prop->on_prop_changed.push([](void* address) -> void {
            Pipeline* pipeline      = reinterpret_cast<Pipeline*>(address);
            RenderPass* render_pass = RenderPass::load_render_pass(pipeline->render_pass);

            size_t count = render_pass ? render_pass->color_attachments.size() : 0;
            pipeline->color_blending.blend_attachment.resize(count);
            pipeline->color_blending.blend_attachment.shrink_to_fit();
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
                             render_pass_prop);
    }
}// namespace Engine
